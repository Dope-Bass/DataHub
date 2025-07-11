using boost::asio::ip::tcp;

class DHClient 
{

public:
    explicit DHClient(const std::string &server, unsigned int port);
    ~DHClient();

    void receiveFile(tcp::socket& socket, const std::string& fileName);
    void sendFile(const std::string& fileName);

    void send_close();

    void console_input_mode();

    bool connect(const std::string& server, unsigned int port);
    bool reconnect();
    
    virtual void process_connection(packet_helpers::connection_status status, size_t clientId);
    virtual void process_getfiles(const packet_helpers::files &files);

private:
    void process_console_input(const std::string& line);
    void process_incoming_packets(const packet_helpers::packet &pack);

    void create_data_packet(packet_helpers::packet &dataPacket, const std::string& fileName,
                            packet_helpers::data& data);

    void requestAllFiles();

    void receive_packet(std::stop_token stoken);
    void send_packet(std::stop_token stoken);

    void push_task(const packet_helpers::packet& pack);

    // Associated server
    std::string m_server;
    unsigned int m_port = -1;

    std::jthread m_commandReadThread;
    std::jthread m_commandWriteThread;

    std::jthread m_dataReadThread;
    std::jthread m_dataWriteThread;

    std::shared_ptr<boost::asio::io_context> sptr_ioContext = nullptr;
    std::shared_ptr<tcp::socket> sptr_socket = nullptr;

    size_t m_id = -1;

    // Syncronization
    std::queue<packet_helpers::packet> m_tasks;
    std::condition_variable m_cv;
    std::mutex m_taskMutex;

    std::mutex m_operationMutex;
};