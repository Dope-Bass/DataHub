#include "../Include/pch.h"
#include "Packet.h"
namespace packet_helpers {

    void deserialize_connection_status(uint8_t* p_buffer, data_var& d)
    {
        auto conn_status = *reinterpret_cast<connection_status*>(p_buffer);
        p_buffer += sizeof(connection_status);

        d.emplace<connection_status>(conn_status);
    }

    void deserialize_data(uint8_t* p_buffer, data_var& d)
    {
        if (p_buffer) {
            data dt;

            dt.strSize = *reinterpret_cast<size_t*>(p_buffer);
            p_buffer += sizeof(dt.strSize);

            dt.fileName = std::string(*p_buffer, sizeof(uint8_t) * dt.strSize);
            p_buffer += dt.fileName.length();

            //memcpy(dt.buffer.data(), p_buffer, DATA_BATCH_SIZE);
            //p_buffer += sizeof(DATA_BATCH_SIZE);

            d.emplace<data>(dt);
        }
    }

    void deserialize_files(uint8_t* p_buffer, data_var& d)
    {
        if (p_buffer) {
            size_t vectorSize = *reinterpret_cast<size_t*>(p_buffer);
            p_buffer += sizeof(vectorSize);

            files files_data;
            files_data.size = vectorSize;
            files_data.data.reserve(vectorSize);

            for (size_t i = 0; i < vectorSize; ++i) {
                size_t strSize = *reinterpret_cast<size_t*>(p_buffer);
                p_buffer += sizeof(strSize);

                files_data.data.emplace_back(std::string((char*)p_buffer, sizeof(uint8_t) * strSize));
                p_buffer += sizeof(uint8_t) * strSize;
            }

            d.emplace<files>(files_data);
        }
    }

    void serialize_connection_status(uint8_t* p_buffer, const data_var& d)
    {
        if (p_buffer) {
            auto conn_data = std::get_if<connection_status>(&d);

            if (conn_data) {
                memcpy((void*)p_buffer, conn_data, sizeof(connection_status));
                p_buffer += sizeof(connection_status);
            }
        }
    }

    void serialize_data(uint8_t* p_buffer, const data_var& d)
    {
    }

    void serialize_files(uint8_t* p_buffer, const data_var& d)
    {
        if (p_buffer) {
            auto files_data = std::get_if<files>(&d);

            if (files_data) {
                size_t vectorSize = files_data->data.size();

                memcpy((void*)p_buffer, &vectorSize, sizeof(vectorSize));
                p_buffer += sizeof(vectorSize);

                for (auto& strFile : files_data->data) {
                    size_t strSize = strFile.size();
                    memcpy((void*)p_buffer, &strSize, sizeof(strSize));
                    p_buffer += sizeof(strSize);

                    memcpy((void*)p_buffer, strFile.data(), sizeof(uint8_t) * strSize);
                    p_buffer += sizeof(uint8_t) * strSize;
                }
            }
        }
    }

    void ph_send_packet(tcp::socket& socket, const packet_helpers::packet &pack)
    {
        std::vector<uint8_t> buffer;
        buffer.resize(pack.size() + sizeof(header));

        ph_set_full_packet(buffer.data(), pack);
    
        boost::asio::write(socket, boost::asio::buffer(buffer.data(), buffer.size()));
    }

    size_t ph_read_packet(tcp::socket& socket, boost::system::error_code& error, packet_helpers::packet& pack)
    {
        uint8_t buffer_header[sizeof(header)];

        // Reading header
        size_t bytes_read = socket.read_some(boost::asio::buffer(buffer_header), error);

        if (error || bytes_read == 0) {
            return bytes_read;
        }

        header hd;
        ph_get_header(buffer_header, hd);

        std::vector<uint8_t> buffer_body;
        buffer_body.resize(hd.size);

        // Reading body
        bytes_read = boost::asio::read(socket, boost::asio::buffer(buffer_body));

        if (error || bytes_read == 0) {
            return bytes_read;
        }

        pack.copy_from_buffer(reinterpret_cast<uint8_t*>(buffer_body.data()));

        return bytes_read;
    }
    void ph_set_full_packet(uint8_t* p_buffer, const packet_helpers::packet& pack)
    {
        if (p_buffer) {
            header hd;
            hd.size = pack.size();

            memcpy((void*)p_buffer, &hd.size, sizeof(hd.size));
            p_buffer += sizeof(hd.size);

            pack.copy_to_buffer(p_buffer);
        }
    }
    void ph_get_header(uint8_t* p_buffer, packet_helpers::header& hd)
    {
        if (p_buffer) {
            hd.size = *reinterpret_cast<size_t*>(p_buffer);
            p_buffer += sizeof(hd.size);
        }
    }
}

size_t createId()
{
    auto uuid = boost::uuids::random_generator()();
    auto strId = boost::uuids::to_string(uuid);

    return std::hash<std::string>{}(strId);
}
