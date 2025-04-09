using boost::asio::ip::tcp;

#include "Connection.h"

class DHServer 
{

public:
    explicit DHServer();
    ~DHServer();

    void receiveFile(tcp::socket& socket, const std::string& fileName);
    void sendFile(tcp::socket& socket, const std::string& fileName);

    void console_input_mode();

    void command(size_t clientId, packet_helpers::packet_type type);

    void closeConnection(size_t clientId);

private:
    void process_console_input(std::string& line);

    void listen_on_connections();

    unsigned int m_port = 27182;

    std::thread m_connectionThread;
    std::thread m_commandThread;

    std::unordered_map<size_t, std::unique_ptr<Connection>> m_clientConnections;

    std::shared_ptr<tcp::acceptor> m_acceptor;

    std::mutex m_mutex;
};