#pragma once

using boost::asio::ip::tcp;

#define PACKET_SIZE 4096

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

    };

#pragma pack(push, 1)
    struct packet {
        packet_type type;
        size_t clientId;

        std::variant<connection_status, data> m_info;

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
                    m_info = (connection_status)*p_buffer;
                    p_buffer += sizeof(connection_status);
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