#include "../Include/pch.h"
namespace packet_helpers {

    void ph_send_packet(tcp::socket& socket, const packet &pack)
    {
        uint8_t buffer[PACKET_SIZE];
        pack.copy_to_buffer(buffer);
    
        boost::asio::write(socket, boost::asio::buffer(buffer, sizeof(buffer)));
    }

}