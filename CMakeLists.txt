cmake_minimum_required(VERSION 3.10)
project(FileTransfer VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)

# SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "C:/boost_1_87_0/boost")
# SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "C:/boost_1_87_0/stage/lib")

set(BOOST_ROOT "C:/Boost")
#set(Boost_USE_STATIC_LIBS OFF) 
#set(Boost_USE_MULTITHREADED ON)  
#set(Boost_USE_STATIC_RUNTIME OFF) 

# Поиск библиотеки Boost
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found! Install Boost and ensure Boost_SYSTEM_LIBRARY is available.")
endif()

# Добавляем пути к общим заголовочным файлам
include_directories(Include)

# Включаем подпроекты
add_subdirectory(Common)
add_subdirectory(Server)
add_subdirectory(Client)

# Дополнительные опции для отладки
option(ENABLE_DEBUG "Enable debug symbols" ON)
if(ENABLE_DEBUG)
    #target_compile_definitions(server PRIVATE DEBUG)
    #target_compile_definitions(client PRIVATE DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()