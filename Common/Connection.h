#pragma once

namespace fs = std::filesystem;

using callback_func = std::function<void(packet_helpers::packet)>;
using disconnect_func = std::function<void(size_t)>;

class Connection
{

public:
	explicit Connection(std::shared_ptr<tcp::socket> socket, 
		size_t id,
		size_t parentId,
		callback_func callback
	);
	~Connection();

	void stop_connection();
	void start_connection();

	void create_connect();

	size_t	get_id()						{ return m_id; }
	size_t	get_receiver()					{ return m_receiverId; }

	void send(const packet_helpers::packet& pack);

private:
	void receive_packet();
	void send_packet();

	void shutdown_socket();

	size_t m_id = -1;

	size_t m_parentId = -1;
	size_t m_receiverId = -1;

	// Handles
	callback_func m_callback;

	// End point address
	std::string m_address;
	uint_least16_t m_port;
	//

	packet_helpers::connection_status m_connection =
		packet_helpers::connection_status::unknown;

	std::atomic_bool m_readRun = false;
	std::atomic_bool m_writeRun = false;

	std::thread m_readThread;
	std::thread m_writeThread;

	// Syncronization
	std::queue<packet_helpers::packet> m_sendQ;
	std::condition_variable m_cv;
	std::mutex m_sendMutex;

	std::shared_ptr<tcp::socket> m_socket;
};

using connection_ptr = std::unique_ptr<Connection>;