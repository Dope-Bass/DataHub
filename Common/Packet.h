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
        get_files,

        unknown
    };

    enum class connection_status : uint8_t {
        established,
        disconnected,
        unknown
    };

    struct data {
        size_t strSize;
        std::string fileName;

        size_t fileSize;
        size_t currentSize;

        char buffer[DATA_BATCH_SIZE];
    };

    using files = std::vector<std::string>;

    using data_var = std::variant<connection_status, 
                                  data, 
                                  files
                                >;

    // Deserialize

    void deserialize_connection_status(uint8_t const* p_buffer, data_var& d);
    void deserialize_data(uint8_t const* p_buffer, data_var& d);
    void deserialize_files(uint8_t const* p_buffer, data_var& d);

    // Serialize

    void serialize_connection_status(uint8_t const* p_buffer, const data_var& d);
    void serialize_data(uint8_t const* p_buffer, const data_var& d);
    void serialize_files(uint8_t const* p_buffer, const data_var& d);

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
            memcpy((void*)p_buffer, &type, sizeof(type));
            p_buffer += sizeof(type);

            memcpy((void*)p_buffer, &clientId, sizeof(clientId));
            p_buffer += sizeof(clientId);

            serialize(p_buffer);
        }

        void serialize(uint8_t const* p_buffer) const
        {
            switch (type) {
                case packet_type::connection: {
                    serialize_connection_status(p_buffer, m_info);
                    return;
                }
                case packet_type::data: {
                    serialize_data(p_buffer, m_info);
                    return;
                }
                case packet_type::get_files: {
                    serialize_files(p_buffer, m_info);
                    return;
                }
                default: {
                    return;
                }
            }
        }

        void deserialize(uint8_t const* p_buffer)
        {
            switch (type) {
                case packet_type::connection: {
                    deserialize_connection_status(p_buffer, m_info);
                    return;
                }
                case packet_type::data: {
                    deserialize_data(p_buffer, m_info);
                    return;
                }
                case packet_type::get_files: {
                    deserialize_files(p_buffer, m_info);
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