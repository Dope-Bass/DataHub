using boost::asio::ip::tcp;

#include "NetProcessor.h"

class DHClient : public NetProcessor
{

public:
    explicit DHClient(const std::string &server, unsigned int port);
    ~DHClient();

    void receiveFile(tcp::socket& socket, const std::string& fileName);
    void sendFile(const std::string& fileName);

    // Executes outside Connection threads
    void command(const packet_helpers::packet packet);

    bool connect(const std::string& server, unsigned int port);
    bool reconnect();
    
    virtual void process_getfiles(const packet_helpers::files &files);

private:
    void create_data_packet(packet_helpers::packet &dataPacket, const std::string& fileName,
                            packet_helpers::data& data);

    virtual bool process_console_input(const std::string& line) override;
    virtual void process_packet(const packet_helpers::packet& pack) override;

    void request_files(size_t connId);

    std::shared_ptr<boost::asio::io_context> sptr_ioContext = nullptr;
};