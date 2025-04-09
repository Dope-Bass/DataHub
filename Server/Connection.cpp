#include "pch.h"
#include "Connection.h"

Connection::Connection(std::shared_ptr<tcp::socket> socket, size_t clientId, 
    std::function<void(size_t, packet_helpers::packet_type)> callback)
{
	m_socket = socket;
    m_clientId = clientId;

    m_serverCallback = callback;

    m_address = socket->remote_endpoint().address().to_string();
    m_port = socket->remote_endpoint().port();

    m_commandReadThread = std::thread([this]() { this->receive_packet(); });
    m_commandReadThread.detach();
    
    m_commandWriteThread = std::thread([this]() { this->send_packet(); });
    m_commandWriteThread.detach();

    create_connection_packet();
}

Connection::~Connection()
{
    std::cout << "Connection destroyed" << std::endl;

    boost::system::error_code error;
    m_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);

    m_socket->close();

    if (m_commandReadThread.joinable()) {
        m_commandReadThread.join();
    }

    if (m_commandWriteThread.joinable()) {
        m_commandWriteThread.join();
    }
}

void Connection::receive_packet()
{
    std::weak_ptr<tcp::socket> sockCheck = m_socket;

    while (true) {
        std::shared_ptr<tcp::socket> sptr = sockCheck.lock();
        if (std::shared_ptr<tcp::socket> sptr = sockCheck.lock()) {
            uint8_t buffer[PACKET_SIZE];

            try {
                boost::system::error_code error;
                size_t bytesRead = m_socket->read_some(boost::asio::buffer(buffer), error);

                if (error || bytesRead == 0) {
                    std::cout << "Client disconnected." << std::endl;
                    break;
                }

                packet_helpers::packet pack;
                pack.copy_from_buffer(buffer);

                std::cout << "Packet received on server " << pack.clientId << std::endl << (int)pack.type << std::endl;

                process_incoming_packet(pack);
            }
            catch (std::exception& e) {
                std::cerr << "Could not read from socket: " << e.what() << std::endl;
            }
        }
        else {
            return;
        }
    }
}

void Connection::create_connection_packet(bool connect)
{
    packet_helpers::packet confirmConnection;
    confirmConnection.type = packet_helpers::packet_type::connection;
    confirmConnection.clientId = m_clientId;

    confirmConnection.m_info = connect
                                ? packet_helpers::connection_status::established
                                : packet_helpers::connection_status::disconnected;

    push_task(confirmConnection);
}

void Connection::push_task(const packet_helpers::packet &pack)
{
    std::unique_lock lk(m_taskMutex);
    m_tasks.push(pack);
    lk.unlock();

    m_cv.notify_all();
}

void Connection::send_packet()
{
    while (true) {
        std::unique_lock lk(m_taskMutex);
        m_cv.wait(lk, [this]() { return !m_tasks.empty(); });

        packet_helpers::packet pack = m_tasks.front();
        m_tasks.pop();

        lk.unlock();

        // Process task

        packet_helpers::ph_send_packet(*m_socket, pack);
    }
}

void Connection::process_incoming_packet(const packet_helpers::packet &pack)
{
    switch (pack.type) {
        case packet_helpers::packet_type::close: {
            m_serverCallback(m_clientId, pack.type);
            return;
        }
        default: {
            std::cout << "Unknown command from client: " << m_clientId << std::endl;
            return;
        }
    }
}