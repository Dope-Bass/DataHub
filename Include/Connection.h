#pragma once

class Connection
{

public:
	explicit Connection(std::shared_ptr<tcp::socket> socket, size_t clientId, std::function<void(size_t)> callback);
	~Connection();

	void create_connection_packet(bool connect = true);

private:
	void receive_packet();
	void send_packet();

	void process_incoming_packet(const packet_helpers::packet& pack);

	void push_task(const packet_helpers::packet &pack);

	size_t m_clientId;

	std::function<void(size_t)> m_serverCallback;

	// End point address
	std::string m_address;
	uint_least16_t m_port;
	//

	std::thread m_commandReadThread;
	std::thread m_commandWriteThread;

	std::thread m_dataReadThread;
	std::thread m_dataWriteThread;

	bool m_operationRunning = false;
	double m_progress = 0.f;

	std::shared_ptr<tcp::socket> m_socket;

	// Syncronization
	std::queue<packet_helpers::packet> m_tasks;
	std::condition_variable m_cv;
	std::mutex m_taskMutex;
};

