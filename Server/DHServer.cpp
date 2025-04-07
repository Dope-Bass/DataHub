#include "pch.h"
#include "DHServer.h"

using boost::asio::ip::tcp;
namespace fs = std::filesystem;

DHServer::DHServer()
{
    m_connectionThread = std::thread([this]() { this->listen_on_connections(); });
    m_connectionThread.detach();
}

DHServer::~DHServer()
{
    for (auto conn : m_clientConnections) {
        conn.second->create_connection_packet(false);
    }
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
            m_connectionThread.join();
        }
    }
}

void DHServer::command(size_t clientId)
{
    std::cout << "Server got command from client!" << clientId;
}

void DHServer::listen_on_connections()
{
    try {
        boost::asio::io_context ioContext;

        m_acceptor.reset(new tcp::acceptor(ioContext, tcp::endpoint(tcp::v4(), m_port)));
        std::cout << "Server set up and listening on port " << m_port << std::endl;

        while (true) {
            auto socket = std::make_shared<tcp::socket>(ioContext);
            m_acceptor->accept(*socket.get());

            std::cout << "Client connected" << std::endl;

            auto uuid = boost::uuids::random_generator()();
            auto strId = boost::uuids::to_string(uuid);
            size_t id = std::hash<std::string>{}(strId);

            using std::placeholders::_1;
            std::function<void(size_t)> callback = std::bind(&DHServer::command, this, _1);

            m_clientConnections.insert({ id,
                std::make_shared<Connection>(socket, id, callback) });

            //m_commandThread = std::thread([this](std::shared_ptr<tcp::socket> socket) { this->getPacket(socket); }, socket);
            //m_commandThread.detach();
            //getPacket(socket);
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
