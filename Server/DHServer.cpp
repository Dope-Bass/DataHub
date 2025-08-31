#include "pch.h"
#include "DHServer.h"

namespace fs = std::filesystem;

DHServer::DHServer()
    : NetProcessor()
{
    m_id = createId();

    m_connectionRun = true;
    m_connectionThread = std::thread([this]() { this->listen_on_connections(); });
}

DHServer::~DHServer()
{
    stop();

    m_connectionRun = false;
    if (m_connectionThread.joinable()) {
        m_connectionThread.join();
    }
}

bool DHServer::process_console_input(const std::string& line)
{
    if (line == "stop") {
        if (m_acceptor->is_open()) {
            m_acceptor->close();
        }

        return false;
    }

    return true;
}

void DHServer::process_packet(const packet_helpers::packet& pack)
{
    switch (pack.type) {
        case packet_helpers::packet_type::connection: {
            process_connection(pack);
            return;
        }
        case packet_helpers::packet_type::get_files: {
            create_getfiles_packet(pack);
            return;
        }
        case packet_helpers::packet_type::data: {
            //process_data_packet(pack);
            return;
        }
        default: {
            std::cout << "Unknown command (sender/receiver): " << pack.senderId << pack.receiverId << std::endl;
            return;
        }
    }
}

void DHServer::create_getfiles_packet(const packet_helpers::packet& pack)
{
    if (hasConnection(pack.senderId)) {
        auto connIt = m_connections.find(pack.senderId);
        packet_helpers::packet get_files;

        get_files.type = packet_helpers::packet_type::get_files;
        get_files.senderId = m_id;
        get_files.receiverId = connIt->second->get_receiver();

        packet_helpers::files files_data;
        //auto str = (*connIt)->get_path().string();
        //files_data.add_file(str);

        for (int index = 0; index < 100; ++index) {
            std::string str = "File Name";
            files_data.add_file(str);
        }

        get_files.m_info = files_data;

        connIt->second->send(pack);
    }
}

void DHServer::listen_on_connections()
{
    try {
        boost::asio::io_context ioContext;

        m_acceptor.reset(new tcp::acceptor(ioContext, tcp::endpoint(tcp::v4(), m_port)));
        std::cout << "Server set up and listening on port " << m_port << std::endl;

        while (m_connectionRun) {
            boost::system::error_code err;
            auto socket = std::make_shared<tcp::socket>(ioContext);
            m_acceptor->accept(*socket.get(), err);

            if (!err) {
                std::cout << "Client connected" << std::endl;
                size_t connId = createId();

                using std::placeholders::_1;
                callback_func callback = std::bind(&DHServer::process_callback, this, _1);

                m_connections.insert({ connId, std::make_unique<Connection>(socket, connId, m_id, callback) });
            }
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

// Функция для приема файла от клиента
void DHServer::receiveFile(tcp::socket& socket, const std::string& fileName) {
    try {
        fs::path fPath("B:\\received_testTransfer.txt");

        std::ofstream outFile(fPath, std::ios::binary);

        char buffer[4096];
        boost::system::error_code error;



        std::cout << "Receiving file \"" << fileName << "\"..." << std::endl;

        while (true) {
            size_t bytesRead = socket.read_some(boost::asio::buffer(buffer), error);
            if (error == boost::asio::error::eof) {
                break; // Конец файла
            }
            if (error) {
                std::cerr << "Error receiving file: " << error.message() << std::endl;
                return;
            }

            outFile.write(buffer, bytesRead);
        }

        outFile.close();
        std::cout << "File \"" << fileName << "\" seccessfully received." << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Exception during reception: " << e.what() << std::endl;
    }
}

// Функция для отправки файла клиенту
void DHServer::sendFile(tcp::socket& socket, const std::string& fileName) {
    try {
        std::ifstream inFile(fileName, std::ios::binary);
        if (!inFile) {
            std::cerr << "Cant open file: " << fileName << std::endl;
            return;
        }

        char buffer[4096];
        std::cout << "Sending file \"" << fileName << "\"..." << std::endl;

        while (inFile.read(buffer, sizeof(buffer))) {
            boost::asio::write(socket, boost::asio::buffer(buffer, inFile.gcount()));
        }

        // Отправляем оставшиеся байты
        if (inFile.gcount() > 0) {
            boost::asio::write(socket, boost::asio::buffer(buffer, inFile.gcount()));
        }

        std::cout << "Файл \"" << fileName << "\" успешно отправлен." << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Исключение при отправке файла: " << e.what() << std::endl;
    }
}