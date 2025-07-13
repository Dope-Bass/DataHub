#pragma once

namespace fs = std::filesystem;

class Connection
{

public:
	explicit Connection(std::shared_ptr<tcp::socket> socket, size_t clientId, fs::path currentDir,
		std::function<void(size_t, packet_helpers::packet_type)> callback);
	~Connection();

	void create_connection_packet(bool connect = true);
	void create_getfiles_packet();

	void push_task(const packet_helpers::packet& pack);

private:
	void receive_packet();
	void send_packet();

	void process_incoming_packet(const packet_helpers::packet& pack);
	void process_data_packet(const packet_helpers::packet& pack);

	void stop_connection();
	void start_connection();

	size_t m_clientId;
	fs::path m_path;

	std::ofstream outFile;

	// Transfer commands assigned to server
	std::function<void(size_t, packet_helpers::packet_type)> m_serverCallback;

	// End point address
	std::string m_address;
	uint_least16_t m_port;
	//

	std::atomic_bool m_readCommandRun = false;
	std::atomic_bool m_writeCommandRun = false;

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

