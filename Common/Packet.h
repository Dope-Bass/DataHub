#pragma once

using boost::asio::ip::tcp;

#define PACKET_SIZE 4096
#define DATA_BATCH_SIZE 2048

namespace packet_helpers {
    enum class packet_type : uint8_t {
        connection = 1,
        close,
        data,
        error,

        unknown
    };

    enum class connection_status : uint8_t {
        established,
        disconnected,
        unknown
    };

    struct data {
        std::string fileName;
        size_t strSize;

        size_t fileSize;
        size_t currentSize;

        char buffer[DATA_BATCH_SIZE];
    };

    using data_var = std::variant<connection_status, data>;

    void deserialize_connection_status(uint8_t const* p_buffer, data_var& d);

    void deserialize_data(uint8_t const* p_buffer, data_var& d);

#pragma pack(push, 1)
    struct packet {
        packet_type type;
        size_t clientId;

        data_var m_info;

        packet() {
            type = packet_type::unknown;
            clientId = 0;
        }

        void copy_from_buffer(uint8_t const* p_buffer)
        {
            type = (packet_type)*p_buffer;    
            p_buffer += sizeof(type);

            clientId = *((size_t*)(p_buffer));
            p_buffer += sizeof(clientId);

            deserialize(p_buffer);
        }

        void copy_to_buffer(uint8_t const* p_buffer) const
        {
            memcpy((void*)p_buffer, this, sizeof(*this));
        }

        void deserialize(uint8_t const* p_buffer)
        {
            switch (type) {
                case packet_type::connection: {
                    deserialize_connection_status(p_buffer, m_info);
                    p_buffer += sizeof(connection_status);
                    return;
                }
                case packet_type::data: {
                    deserialize_data(p_buffer, m_info);
                    p_buffer += sizeof(data);
                    return;
                }
                default: {
                    return;
                }
            }
        }
    };
#pragma pack(pop)

    void ph_send_packet(tcp::socket& socket, const packet &pack);
};