#include "pch.h"  // Включение предкомпилированных заголовков
#include "DHClient.h"

int main(int argc, char* argv[]) {
    DHClient* client = new DHClient("127.0.0.1", 27182);

    client->console_input_mode();

    try {
        //if (argc != 4) {
        //    std::cerr << "Использование: client <адрес_сервера> <порт> <имя_файла>" << std::endl;
        //    return -1;
        //}

        //std::string serverAddress = argv[1];  // Адрес сервера
        //unsigned short portNumber = static_cast<unsigned short>(std::stoi(argv[2]));  // Порт сервера
        //std::string fileName = argv[3];  // Имя файла для отправки

        //std::string server = "127.0.0.1";
        //unsigned int port = 8080;
        //std::string file = "B:\\testTransfer.txt";

        //boost::asio::io_context ioContext;

        //// Создаем сокет для подключения к серверу
        //tcp::resolver resolver(ioContext);
        //tcp::resolver::results_type endpoints = resolver.resolve(server, std::to_string(port));
        //tcp::socket socket(ioContext);

        // Подключаемся к серверу
        //boost::asio::connect(socket, endpoints);
        std::cout << "Server connection." << std::endl;

        // Отправляем файл на сервер
        //client->sendFile(socket, file);

        // Получаем файл обратно от сервера
        //std::string receivedFileName = "received_" + file;
        //client->receiveFile(socket, receivedFileName);

        //socket.close();
        std::cout << "Connection closed." << std::endl;

    } catch (std::exception& e) {
        std::cerr << "Исключение: " << e.what() << std::endl;
    }

    return 0;
}