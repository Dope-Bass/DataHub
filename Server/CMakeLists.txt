set(CMAKE_CXX_STANDARD 23)

# Серверный CMakeLists.txt
set(SERVER_SOURCES
    DHServer.cpp
    main.cpp
)

# Создаем исполняемый файл сервера
add_executable(Server ${SERVER_SOURCES})

# Указываем предкомпилированные заголовки
target_precompile_headers(Server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../Include/pch.h)

# Добавляем пути к заголовочным файлам
target_include_directories(Server PRIVATE ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

# Ссылаемся на необходимые библиотеки
target_link_libraries(Server Boost::system Threads::Threads NetCommon)