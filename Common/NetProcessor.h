#pragma once

#include "Connection.h"

#define WORKERS 2

class NetProcessor 
{
public:
	explicit NetProcessor();
	~NetProcessor();

	bool hasConnection(size_t connId);
	
	void start();
	void stop(size_t connId);
	void stop();

	void console_input_mode();

	virtual void process_packet(const packet_helpers::packet& pack) = 0;
	virtual bool process_console_input(const std::string& line) = 0;
	virtual void process_connection(const packet_helpers::packet& pack);

	void process_callback(const packet_helpers::packet& pack);

protected:
	void push_task(const packet_helpers::packet& pack);

	void processor_thread();

	std::unordered_map<size_t, connection_ptr> m_connections;

	std::vector<std::thread> m_workers;

	// Syncronization
	std::queue<packet_helpers::packet> m_tasks;
	std::condition_variable m_cv;
	std::mutex m_taskMutex;

	std::atomic_bool m_processRun = false;

	size_t m_id = -1;
};