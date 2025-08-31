#include "pch.h"
#include "Connection.h"

Connection::Connection(std::shared_ptr<tcp::socket> socket, 
    size_t id,
    size_t parentId,
    callback_func callback)
{
    m_id = id;
    m_parentId = parentId;

	m_socket = socket;

    m_callback = callback;

    m_address = socket->remote_endpoint().address().to_string();
    m_port = socket->remote_endpoint().port();

    start_connection();
    create_connect();
}

Connection::~Connection()
{
    std::cout << "Connection destroyed" << std::endl;

    stop_connection();
}

void Connection::send(const packet_helpers::packet& pack)
{
    {
        std::lock_guard lk(m_sendMutex);
        m_sendQ.push(pack);
    }

    m_cv.notify_all();
}

void Connection::receive_packet()
{
    while (m_readRun) {
        try {
            packet_helpers::packet pack;
            boost::system::error_code error;

            size_t bytesRead = packet_helpers::ph_read_packet(*m_socket, error, pack);

            if (error || bytesRead == 0) {
                std::cout << "Socket destroyed. Disconnection" << std::endl;

                if (m_connection != packet_helpers::connection_status::disconnected) {
                    m_connection = packet_helpers::connection_status::disconnected;

                    pack.senderId       = m_id;
                    pack.type           = packet_helpers::packet_type::connection;
                    pack.m_info         = m_connection;
                
                    m_callback(pack);
                }
                
                break;
            }

            std::cout << "Packet received (sender/receiver) " << pack.senderId << " " << pack.receiverId << std::endl << (int)pack.type << std::endl;

            if (pack.type == packet_helpers::packet_type::connection) {
                m_connection = packet_helpers::connection_status::established;
                m_receiverId = pack.senderId;
            }

            pack.senderId = m_id;
            m_callback(pack);
        }
        catch (std::exception& e) {
            std::cerr << "Could not read from socket: " << e.what() << std::endl;
        }
    }
}

void Connection::send_packet()
{
    while (m_writeRun) {
        std::unique_lock lk(m_sendMutex);
        m_cv.wait(lk, [this]() { return !m_sendQ.empty(); });

        packet_helpers::packet pack = m_sendQ.front();
        m_sendQ.pop();

        lk.unlock();

        packet_helpers::ph_send_packet(*m_socket, pack);
    }
}

void Connection::shutdown_socket()
{
    boost::system::error_code error;
    m_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);

    m_socket->close();
}

void Connection::stop_connection()
{
    if (!m_readRun && !m_writeRun) {
        return;
    }

    m_connection = packet_helpers::connection_status::disconnected;

    m_readRun = false;
    m_writeRun = false;

    shutdown_socket();

    m_cv.notify_all();

    if (m_readThread.joinable()) {
        m_readThread.join();
    }

    if (m_writeThread.joinable()) {
        m_writeThread.join();
    }
}

void Connection::start_connection()
{
    m_readRun = true;
    m_writeRun = true;

    m_readThread = std::thread([this]() { this->receive_packet(); });
    m_writeThread = std::thread([this]() { this->send_packet(); });
}

void Connection::create_connect()
{
    packet_helpers::packet confirmConnection;
    confirmConnection.type = packet_helpers::packet_type::connection;
    confirmConnection.senderId = m_parentId;

    confirmConnection.m_info = packet_helpers::connection_status::established;

    send(confirmConnection);
}