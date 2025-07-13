#include "pch.h"
#include "DHServer.h"

using boost::asio::ip::tcp;
namespace fs = std::filesystem;

DHServer::DHServer()
{
    start_connection();
}

DHServer::~DHServer()
{
    for (auto &conn : m_clientConnections) {
        conn.second->create_connection_packet(false);
    }

    stop_connection();
}

void DHServer::console_input_mode()
{
    for (std::string line; std::cout << "Command > " && std::getline(std::cin, line); )
    {
        if (!line.empty()) {
            std::cout << "Process" << std::endl;
            process_console_input(line);
        }
    }
}

void DHServer::process_console_input(std::string& line)
{
    if (line == "stop") {
        if (m_acceptor->is_open()) {
            m_acceptor->close();

            stop_connection();
        }
    }
}

void DHServer::command(size_t clientId, packet_helpers::packet_type type)
{
    std::cout << "Server got command from client! - " << clientId << std::endl;

    switch (type) {
        case packet_helpers::packet_type::close: {
            closeConnection(clientId);
            break;
        }
        default: {
            return;
        }
    }
}

void DHServer::closeConnection(size_t clientId)
{
    std::lock_guard lock(m_mutex);

    auto connectionIt = m_clientConnections.find(clientId);
    if (connectionIt != m_clientConnections.end()) {
        m_clientConnections.erase(connectionIt);
    }
}

void DHServer::listen_on_connections()
{
    try {
        boost::asio::io_context ioContext;

        m_acceptor.reset(new tcp::acceptor(ioContext, tcp::endpoint(tcp::v4(), m_port)));
        std::cout << "Server set up and listening on port " << m_port << std::endl;

        while (true) {
            if (!m_connectionRun) {
                break;
            }

            auto socket = std::make_shared<tcp::socket>(ioContext);
            m_acceptor->accept(*socket.get());

            std::cout << "Client connected" << std::endl;

            auto uuid = boost::uuids::random_generator()();
            auto strId = boost::uuids::to_string(uuid);
            size_t id = std::hash<std::string>{}(strId);

            using std::placeholders::_1;
            using std::placeholders::_2;
            std::function<void(size_t, packet_helpers::packet_type)> callback = std::bind(&DHServer::command, this, _1, _2);

            m_clientConnections.insert({ id,
                std::make_unique<Connection>(socket, id, fs::current_path(), callback)});
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

void DHServer::stop_connection()
{
    m_connectionRun = false;
    if (m_connectionThread.joinable()) {
        m_connectionThread.join();
    }
}

void DHServer::start_connection()
{
    m_connectionRun = true;
    m_connectionThread = std::thread([this]() { this->listen_on_connections(); });
}