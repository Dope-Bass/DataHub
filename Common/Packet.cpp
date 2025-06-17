#include "../Include/pch.h"
namespace packet_helpers {

    void deserialize_connection_status(uint8_t const* p_buffer, data_var& d)
    {
        auto conn_status = (connection_status)*p_buffer;
        p_buffer += sizeof(connection_status);

        d.emplace<connection_status>(conn_status);
    }

    void deserialize_data(uint8_t const* p_buffer, data_var& d)
    {
        data dt;

        dt.strSize = (size_t)*p_buffer;
        p_buffer += sizeof(size_t);

        dt.fileName = std::string(*p_buffer, sizeof(uint8_t) * dt.strSize);
        p_buffer += dt.fileName.length();

        memcpy(dt.buffer, p_buffer, DATA_BATCH_SIZE);
        p_buffer += sizeof(DATA_BATCH_SIZE);

        d.emplace<data>(dt);
    }

    void deserialize_files(uint8_t const* p_buffer, data_var& d)
    {
        size_t vectorSize = (size_t)*p_buffer;
        p_buffer += sizeof(size_t);

        std::vector<std::string> vecString;
        vecString.reserve(vectorSize);

        for (size_t i = 0; i < vectorSize; ++i) {
            size_t strSize = (size_t)*p_buffer;
            p_buffer += sizeof(size_t);

            vecString.emplace_back(std::string((char*)p_buffer, sizeof(uint8_t) * strSize));
            p_buffer += sizeof(uint8_t) * strSize;
        }

        d.emplace<files>(vecString);
    }

    void serialize_connection_status(uint8_t const* p_buffer, const data_var& d)
    {
        memcpy((void*)p_buffer, &std::get<connection_status>(d), sizeof(connection_status));
        p_buffer += sizeof(connection_status);
    }

    void serialize_data(uint8_t const* p_buffer, const data_var& d)
    {
    }

    void serialize_files(uint8_t const* p_buffer, const data_var& d)
    {
        size_t vectorSize = std::get<files>(d).size();
        memcpy((void*)p_buffer, &vectorSize, sizeof(size_t));
        p_buffer += sizeof(size_t);

        for (auto& strFile : std::get<files>(d)) {
            size_t strSize = strFile.size();
            memcpy((void*)p_buffer, &strSize, sizeof(size_t));
            p_buffer += sizeof(size_t);

            memcpy((void*)p_buffer, strFile.data(), sizeof(uint8_t) * strSize);
            p_buffer += sizeof(uint8_t) * strSize;
        }
    }

    void ph_send_packet(tcp::socket& socket, const packet &pack)
    {
        uint8_t buffer[PACKET_SIZE];
        pack.copy_to_buffer(buffer);
    
        boost::asio::write(socket, boost::asio::buffer(buffer, sizeof(buffer)));
    }

}