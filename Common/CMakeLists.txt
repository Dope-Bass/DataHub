set(CMAKE_CXX_STANDARD 23)

add_library(NetCommon "")

target_sources(NetCommon
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Packet.cpp
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/Packet.h
)

target_include_directories(NetCommon
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(NetCommon Boost::system Threads::Threads)