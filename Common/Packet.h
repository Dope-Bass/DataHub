#pragma once

using boost::asio::ip::tcp;

#define PACKET_SIZE 4096
#define DATA_BATCH_SIZE 2048

size_t createId();

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

        std::vector<uint8_t> buffer;
    };

    struct files {
        size_t size = 0;
        std::vector<std::string> data;

        void add_file(std::string& str)
        {
            data.push_back(str);
            size += str.size();
            size += sizeof(size_t);
        }
    };

    using data_var = std::variant<connection_status, 
                                  data, 
                                  files
                                >;

    // Deserialize

    void deserialize_connection_status(uint8_t* p_buffer, data_var& d);
    void deserialize_data(uint8_t* p_buffer, data_var& d);
    void deserialize_files(uint8_t* p_buffer, data_var& d);

    // Serialize

    void serialize_connection_status(uint8_t* p_buffer, const data_var& d);
    void serialize_data(uint8_t* p_buffer, const data_var& d);
    void serialize_files(uint8_t* p_buffer, const data_var& d);

#pragma pack(push, 1)
    struct header {
        size_t size;
    };

    struct packet {
        packet_type type;
        size_t senderId;
        size_t receiverId;

        data_var m_info;

        packet() {
            type = packet_type::unknown;
            senderId = -1;
            receiverId = -1;
        }

        size_t size() const
        {
            auto size = sizeof(packet);

            switch (type) {
                case packet_type::connection: {
                    return size;
                }
                case packet_type::data: {
                    return size;
                }
                case packet_type::get_files: {
                    auto files_data = std::get_if<files>(&m_info);

                    if (files_data) {
                        size += sizeof(files_data->data.size());     // vector size
                        size += files_data->size;   // vector data size
                    }
                    return size;
                }
                default: {
                    return size;
                }
            }

            return size;
        }

        void copy_from_buffer(uint8_t* p_buffer)
        {
            type = *reinterpret_cast<packet_type*>(p_buffer);
            p_buffer += sizeof(type);

            senderId = *reinterpret_cast<size_t*>(p_buffer);
            p_buffer += sizeof(senderId);
            
            receiverId = *reinterpret_cast<size_t*>(p_buffer);
            p_buffer += sizeof(receiverId);

            deserialize(p_buffer);
        }

        void copy_to_buffer(uint8_t* p_buffer) const
        {
            memcpy((void*)p_buffer, &type, sizeof(type));
            p_buffer += sizeof(type);

            memcpy((void*)p_buffer, &senderId, sizeof(senderId));
            p_buffer += sizeof(senderId);
            
            memcpy((void*)p_buffer, &receiverId, sizeof(receiverId));
            p_buffer += sizeof(receiverId);

            serialize(p_buffer);
        }

        void serialize(uint8_t* p_buffer) const
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

        void deserialize(uint8_t* p_buffer)
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
    size_t ph_read_packet(tcp::socket& socket, boost::system::error_code& error, packet_helpers::packet &pack);

    void ph_set_full_packet(uint8_t* p_buffer, const packet_helpers::packet& pack);
    void ph_get_header(uint8_t* p_buffer, packet_helpers::header& hd);
};