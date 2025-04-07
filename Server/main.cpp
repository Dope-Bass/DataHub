
#include "pch.h"
#include "DHServer.h"

int main() {
    auto srv = std::make_unique<DHServer>();

    try {
        srv->console_input_mode();

        //boost::asio::io_context ioContext;

        //tcp::acceptor acceptor(ioContext, tcp::endpoint(tcp::v4(), 8080));
        //std::cout << "Server set up and listening on port 8080" << std::endl;

        //while (true) {
        //    tcp::socket socket(ioContext);
        //    acceptor.accept(socket);

        //    std::cout << "Client connected." << std::endl;

        //    // Получаем имя файла от клиента
        //    char fileNameBuffer[256];
        //    boost::system::error_code error;
        //    size_t bytesRead = socket.read_some(boost::asio::buffer(fileNameBuffer), error);
        //    if (error) {
        //        std::cerr << "Error getting filename: " << error.message() << std::endl;
        //        continue;
        //    }

        //    std::string fileName(fileNameBuffer, bytesRead);
        //    fileName.erase(fileName.find_last_not_of(" \n\r\t") + 1); // Удаляем лишние пробелы

        //    if (fileName.empty()) {
        //        std::cerr << "Filename is empty!" << std::endl;
        //        continue;
        //    }

        //    //std::cout << "Клиент хочет отправить файл: " << fileName << std::endl;

        //    // Принимаем файл от клиента
        //    srv->receiveFile(socket, fileName);

        //    // Отправляем файл обратно клиенту
        //    //std::cout << "Отправка файла обратно клиенту..." << std::endl;
        //    //srv->sendFile(socket, fileName);

        //    socket.close();
        //}
    } catch (std::exception& e) {
        std::cerr << "Исключение: " << e.what() << std::endl;
    }

    return 0;
}