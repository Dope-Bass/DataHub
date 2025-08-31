#include "pch.h"
#include "DHClient.h"

DHClient::DHClient(const std::string& server, unsigned int port)
    : NetProcessor()
{
    std::string file = "B:\\testTransfer.txt";

    sptr_ioContext.reset(new boost::asio::io_context);

    connect(server, port);
}

DHClient::~DHClient() 
{
    std::cout << "Client destroyed" << std::endl;

    stop();
}

bool DHClient::process_console_input(const std::string& line)
{
    if (line == "close") {
        stop();
    }
    else if (line == "reconnect") {
        reconnect();
    }
    else if (line == "get_files") {
        request_files(m_connections.begin()->second->get_id());
    }
    else {
        sendFile(line);
    }

    return true;
}

bool DHClient::connect(const std::string& server, unsigned int port)
{
    // Create socket
    boost::system::error_code err;
    tcp::resolver resolver(*sptr_ioContext);
    tcp::resolver::results_type endpoints = resolver.resolve(server, std::to_string(port), err);

    if (!err) {
        auto socket = make_shared<tcp::socket>(*sptr_ioContext);

        // Connect to server
        try {
            boost::asio::connect(*socket, endpoints, err);

            if (!err) {
                size_t connId = createId();

                using std::placeholders::_1;
                callback_func callback = std::bind(&DHClient::process_callback, this, _1);

                m_connections.insert({ connId, std::make_unique<Connection>(socket, connId, m_id, callback) });

                return true;
            }
            
            return false;
        }
        catch (std::exception& e) {
            std::cerr << "Client could not connect: " << e.what() << std::endl;

            return false;
        }
    }

    return false;
}

bool DHClient::reconnect()
{
    //stop_connection();

    //boost::system::error_code error;
    //sptr_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);

    //return connect(m_server, m_port);

    return false;
}

void DHClient::process_packet(const packet_helpers::packet& pack)
{
    switch (pack.type) {
        case packet_helpers::packet_type::connection: {
            process_connection(pack);
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

void DHClient::process_getfiles(const packet_helpers::files& files)
{
    for (auto& strFile : files.data) {
        std::cout << "Received file: " << strFile << std::endl;
    }
}

void DHClient::create_data_packet(packet_helpers::packet &dataPacket, const std::string &fileName,
                                    packet_helpers::data &data)
{
    //data.strSize = fileName.length();
    //data.fileName = fileName;

    //dataPacket.m_info = data;
    //dataPacket.type = packet_helpers::packet_type::data;
    //dataPacket.clientId = m_id;
}

void DHClient::request_files(size_t connId)
{
    if (hasConnection(connId)) {
        packet_helpers::packet getFiles;
        getFiles.type = packet_helpers::packet_type::get_files;

        packet_helpers::files vecFiles;
        getFiles.m_info = vecFiles;

        auto connIt = m_connections.find(connId);
        connIt->second->send(getFiles);
    }
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