#include "pch.h"
#include "NetProcessor.h"

NetProcessor::NetProcessor()
{
	m_id = createId();

    m_processRun = true;

    for (int index = 0; index < WORKERS; ++index) {
        m_workers.emplace_back([this]() { this->processor_thread(); });
    }
}

NetProcessor::~NetProcessor()
{
    stop();

    m_processRun = false;

    m_cv.notify_all();

    for (auto& thread : m_workers) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

bool NetProcessor::hasConnection(size_t connId)
{
    return m_connections.contains(connId);
}

void NetProcessor::start()
{

}

void NetProcessor::stop(size_t connId)
{
    if (hasConnection(connId)) {
        auto connIt = m_connections.find(connId);
        m_connections.erase(connId);
    }
}

void NetProcessor::stop()
{
	m_connections.clear();
}

void NetProcessor::push_task(const packet_helpers::packet& pack)
{
    {
        std::lock_guard lk(m_taskMutex);
        m_tasks.push(pack);
    }

    m_cv.notify_all();
}

void NetProcessor::processor_thread()
{
    while (m_processRun) {
        std::unique_lock lk(m_taskMutex);
        m_cv.wait(lk, [this]() { return !m_processRun || !m_tasks.empty(); });

        if (!m_processRun) {
            lk.unlock();
            break;
        }

        packet_helpers::packet pack = m_tasks.front();
        m_tasks.pop();

        lk.unlock();

        process_packet(pack);
    }
}

void NetProcessor::process_callback(const packet_helpers::packet& pack)
{
    push_task(pack);
}

void NetProcessor::console_input_mode()
{
    for (std::string line; std::cout << "Command > " && std::getline(std::cin, line); )
    {
        if (!line.empty()) {
            std::cout << "Process" << std::endl;
            if (!process_console_input(line)) {
                return;
            }
        }
    }
}

void NetProcessor::process_connection(const packet_helpers::packet& pack)
{
    auto connection = std::get<packet_helpers::connection_status>(pack.m_info);

    switch (connection) {
    case packet_helpers::connection_status::disconnected: {
        stop(pack.senderId);
        return;
    }
    default:
        return;
    }
}
