#include "../Include/pch.h"
namespace packet_helpers {

    void deserialize_connection_status(uint8_t const* p_buffer, data_var& d)
    {
        auto conn_status = (connection_status)*p_buffer;

        d.emplace<connection_status>(conn_status);
    }

    void deserialize_data(uint8_t const* p_buffer, data_var& d)
    {
        data dt;

        memcpy(dt.buffer, p_buffer, DATA_BATCH_SIZE);

        d.emplace<data>(dt);
    }

    void ph_send_packet(tcp::socket& socket, const packet &pack)
    {
        uint8_t buffer[PACKET_SIZE];
        pack.copy_to_buffer(buffer);
    
        boost::asio::write(socket, boost::asio::buffer(buffer, sizeof(buffer)));
    }

}