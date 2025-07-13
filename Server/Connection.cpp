#include "pch.h"
#include "Connection.h"

Connection::Connection(std::shared_ptr<tcp::socket> socket, size_t clientId, fs::path currentDir,
    std::function<void(size_t, packet_helpers::packet_type)> callback)
{
	m_socket = socket;
    m_clientId = clientId;

    m_serverCallback = callback;

    m_path = currentDir;

    m_address = socket->remote_endpoint().address().to_string();
    m_port = socket->remote_endpoint().port();

    start_connection();

    create_connection_packet();
}

Connection::~Connection()
{
    std::cout << "Connection destroyed" << std::endl;

    boost::system::error_code error;
    m_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);

    m_socket->close();

    stop_connection();
}

void Connection::receive_packet()
{
    std::weak_ptr<tcp::socket> sockCheck = m_socket;

    while (true) {
        if (!m_readCommandRun) {
            break;
        }

        std::shared_ptr<tcp::socket> sptr = sockCheck.lock();
        if (std::shared_ptr<tcp::socket> sptr = sockCheck.lock()) {
            uint8_t buffer[PACKET_SIZE];

            try {
                packet_helpers::packet pack;
                boost::system::error_code error;
                size_t bytesRead = packet_helpers::ph_read_packet(*sptr, error, pack);

                if (error || bytesRead == 0) {
                    std::cout << "Client disconnected." << std::endl;
                    break;
                }

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

void Connection::create_getfiles_packet()
{
    packet_helpers::packet get_files;
    get_files.type = packet_helpers::packet_type::get_files;
    get_files.clientId = m_clientId;

    packet_helpers::files files_data;
    auto str = m_path.string();
    files_data.add_file(str);

    get_files.m_info = files_data;

    push_task(get_files);
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
        if (!m_writeCommandRun) {
            break;
        }

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
        case packet_helpers::packet_type::get_files: {
            create_getfiles_packet();
            return;
        }
        case packet_helpers::packet_type::close: {
            m_serverCallback(m_clientId, pack.type);
            return;
        }
        case packet_helpers::packet_type::data: {
            process_data_packet(pack);
            return;
        }
        default: {
            std::cout << "Unknown command from client: " << m_clientId << std::endl;
            return;
        }
    }
}

void Connection::process_data_packet(const packet_helpers::packet& pack)
{
    //try {
    //    auto data = std::get<packet_helpers::data>(pack.m_info);

    //    fs::path fPath(data.fileName);

    //    if (!outFile.is_open()) {
    //        outFile.open(fPath, std::ios::binary);
    //    }

    //    //char buffer[4096];
    //    //boost::system::error_code error;



    //    std::cout << "Receiving file \"" << std::get<packet_helpers::data>(pack.m_info).fileName << "\"..." << std::endl;

    //    //while (true) {
    //    //    size_t bytesRead = socket.read_some(boost::asio::buffer(buffer), error);
    //    //    if (error == boost::asio::error::eof) {
    //    //        break; // Конец файла
    //    //    }
    //    //    if (error) {
    //    //        std::cerr << "Error receiving file: " << error.message() << std::endl;
    //    //        return;
    //    //    }

    //    outFile.write(data.buffer, DATA_BATCH_SIZE);
    //    //}

    //    //outFile.close();
    //    if (data.currentSize >= data.fileSize) {
    //        outFile.close();
    //        std::cout << "File \"" << data.fileName << "\" seccessfully received." << std::endl;
    //    }
    //} catch (std::exception& e) {
    //    std::cerr << "Exception during reception: " << e.what() << std::endl;
    //    outFile.close();
    //}
}

void Connection::stop_connection()
{
    m_readCommandRun = false;
    m_writeCommandRun = false;

    if (m_commandReadThread.joinable()) {
        m_commandReadThread.join();
    }

    if (m_commandWriteThread.joinable()) {
        m_commandWriteThread.join();
    }
}

void Connection::start_connection()
{
    m_readCommandRun = true;
    m_writeCommandRun = true;

    m_commandReadThread = std::thread([this]() { this->receive_packet(); });
    m_commandWriteThread = std::thread([this]() { this->send_packet(); });
}