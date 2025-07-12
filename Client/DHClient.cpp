#include "pch.h"
#include "DHClient.h"

DHClient::DHClient(const std::string& server, unsigned int port)
{
    std::string file = "B:\\testTransfer.txt";

    sptr_ioContext.reset(new boost::asio::io_context);

    connect(server, port);
}

DHClient::~DHClient() 
{
    std::cout << "Client destroyed" << std::endl;
    sptr_socket->close();

    m_commandReadThread.request_stop();
    m_commandWriteThread.request_stop();
}

void DHClient::process_console_input(const std::string& line)
{
    if (line == "close") {
        send_close();
    }
    else if (line == "reconnect") {
        reconnect();
    }
    else if (line == "get_files") {
        requestAllFiles();
    }
    else {
        sendFile(line);
    }
}

bool DHClient::connect(const std::string& server, unsigned int port)
{
    std::lock_guard lock(m_operationMutex);

    m_server = server;
    m_port = port;

    // Create socket
    tcp::resolver resolver(*sptr_ioContext);
    tcp::resolver::results_type endpoints = resolver.resolve(server, std::to_string(port));

    sptr_socket.reset(new tcp::socket(*sptr_ioContext));

    // Connect to server
    try {
        boost::asio::connect(*sptr_socket, endpoints);

        m_commandReadThread = std::jthread([this](std::stop_token stoken) { this->receive_packet(stoken); });
        m_commandWriteThread = std::jthread([this](std::stop_token stoken) { this->send_packet(stoken); });

        return true;
    }
    catch (std::exception& e) {
        std::cerr << "Client could not connect: " << e.what() << std::endl;

        return false;
    }
}

bool DHClient::reconnect()
{
    return connect(m_server, m_port);
}

void DHClient::receive_packet(std::stop_token stoken)
{
    while (true) {
        if (stoken.stop_requested()) {
            break;
        }

        packet_helpers::packet pack;
        boost::system::error_code error;
        size_t bytesRead = packet_helpers::ph_read_packet(*sptr_socket, error, pack);

        if (error || bytesRead == 0) {
            std::cout << "Server disconnected" << std::endl;

            process_connection(packet_helpers::connection_status::disconnected, -1);
            break;
        }

        std::cout << "Packet received from server " << pack.clientId << std::endl << (int)pack.type << std::endl;

        process_incoming_packets(pack);
    }
}

void DHClient::process_incoming_packets(const packet_helpers::packet& pack)
{
    switch (pack.type) {
        case packet_helpers::packet_type::connection: {
            process_connection(std::get<packet_helpers::connection_status>(pack.m_info), pack.clientId);
            return;
        }
        case packet_helpers::packet_type::data: {
            // Process packet with data
            return;
        }
        case packet_helpers::packet_type::get_files: {
            process_getfiles(std::get<packet_helpers::files>(pack.m_info));
            return;
        }
        default: {
            return;
        }
    }
}

void DHClient::process_connection(packet_helpers::connection_status status, size_t clientId)
{
    std::cout << "Received connection status: " << (int)status << std::endl;
    m_id = clientId;
}

void DHClient::process_getfiles(const packet_helpers::files& files)
{
    for (auto& strFile : files.data) {
        std::cout << "Received file: " << strFile << std::endl;
    }
}

void DHClient::send_packet(std::stop_token stoken)
{
    while (true) {
        if (stoken.stop_requested()) {
            break;
        }

        std::unique_lock lk(m_taskMutex);
        m_cv.wait(lk, [this]() { return !m_tasks.empty(); });

        packet_helpers::packet pack = m_tasks.front();
        m_tasks.pop();

        lk.unlock();

        // Process task

        packet_helpers::ph_send_packet(*sptr_socket, pack);
    }
}

void DHClient::console_input_mode()
{
    for (std::string line; std::cout << "Command > " && std::getline(std::cin, line); )
    {
        if (!line.empty()) { 
            std::cout << "Process" << std::endl;
            process_console_input(line);
        }
    }
}

void DHClient::send_close()
{
    packet_helpers::packet closeConnection;
    closeConnection.type = packet_helpers::packet_type::close;
    closeConnection.clientId = m_id;

    push_task(closeConnection);
}

void DHClient::create_data_packet(packet_helpers::packet &dataPacket, const std::string &fileName,
                                    packet_helpers::data &data)
{
    data.strSize = fileName.length();
    data.fileName = fileName;

    dataPacket.m_info = data;
    dataPacket.type = packet_helpers::packet_type::data;
    dataPacket.clientId = m_id;
}

void DHClient::requestAllFiles()
{
    packet_helpers::packet getFiles;
    getFiles.type = packet_helpers::packet_type::get_files;
    getFiles.clientId = m_id;

    packet_helpers::files vecFiles;
    getFiles.m_info = vecFiles;

    push_task(getFiles);
}

void DHClient::sendFile(const std::string& fileName) {
    //try {
    //    std::ifstream inFile(fileName, std::ios::binary);
    //    if (!inFile) {
    //        std::cerr << "Cant open file: " << fileName << std::endl;
    //        return;
    //    }

    //    // Отправляем имя файла серверу
    //    //boost::asio::write(socket, boost::asio::buffer(fileName.c_str(), fileName.size() + 1));

    //    //char buffer[DATA_BATCH_SIZE];
    //    std::cout << "Sending file \"" << fileName << "\"..." << std::endl;


    //    packet_helpers::packet dataPacket;
    //    packet_helpers::data data;
    //    while (inFile.read(data.buffer, sizeof(data.buffer))) {
    //        //boost::asio::write(socket, boost::asio::buffer(buffer, inFile.gcount()));
    //        
    //        create_data_packet(dataPacket, fileName, data);
    //        push_task(dataPacket);
    //    }

    //    // Sending remain bytes
    //    if (inFile.gcount() > 0) {
    //        //boost::asio::write(socket, boost::asio::buffer(buffer, inFile.gcount()));
    //        inFile.read(data.buffer, inFile.gcount());

    //        create_data_packet(dataPacket, fileName, data);
    //        push_task(dataPacket);
    //    }

    //    std::cout << "File \"" << fileName << "\" sended." << std::endl;
    //} catch (std::exception& e) {
    //    std::cerr << "Exception: " << e.what() << std::endl;
    //}
}

void DHClient::receiveFile(boost::asio::ip::tcp::socket& socket, const std::string& fileName) {
    try {
        std::ofstream outFile(fileName, std::ios::binary);
        if (!outFile) {
            std::cerr << "Не удалось создать файл: " << fileName << std::endl;
            return;
        }

        char buffer[4096];
        boost::system::error_code error;

        std::cout << "Прием файла \"" << fileName << "\"..." << std::endl;

        while (true) {
            size_t bytesRead = socket.read_some(boost::asio::buffer(buffer), error);
            if (error == boost::asio::error::eof) {
                break; // Конец файла
            }
            if (error) {
                std::cerr << "Ошибка при приеме файла: " << error.message() << std::endl;
                return;
            }

            outFile.write(buffer, bytesRead);
        }

        outFile.close();
        std::cout << "Файл \"" << fileName << "\" успешно принят." << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Исключение при приеме файла: " << e.what() << std::endl;
    }
}

void DHClient::push_task(const packet_helpers::packet& pack)
{
    std::unique_lock lk(m_taskMutex);
    m_tasks.push(pack);
    lk.unlock();

    m_cv.notify_all();
}