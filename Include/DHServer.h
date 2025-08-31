using boost::asio::ip::tcp;

#include "NetProcessor.h"

class DHServer : public NetProcessor
{

public:
    explicit DHServer();
    ~DHServer();

    void receiveFile(tcp::socket& socket, const std::string& fileName);
    void sendFile(tcp::socket& socket, const std::string& fileName);

private:
    virtual bool process_console_input(const std::string& line) override;
    virtual void process_packet(const packet_helpers::packet& pack) override;

    void listen_on_connections();

    void create_getfiles_packet(const packet_helpers::packet& pack);

    // -------------------------

    unsigned int m_port = 27182;
    size_t m_id = -1;

    std::atomic_bool m_connectionRun = false;

    std::thread m_connectionThread;

    std::shared_ptr<tcp::acceptor> m_acceptor;

    std::mutex m_mutex;
};