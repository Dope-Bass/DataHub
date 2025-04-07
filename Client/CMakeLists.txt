set(CMAKE_CXX_STANDARD 23)

# Клиентский CMakeLists.txt
set(CLIENT_SOURCES
    DHClient.cpp
    main.cpp
)

# Создаем исполняемый файл клиента
add_executable(Client ${CLIENT_SOURCES})

# Указываем предкомпилированные заголовки
target_precompile_headers(Client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../Include/pch.h)

# Добавляем пути к заголовочным файлам
target_include_directories(Client PRIVATE ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

# Ссылаемся на необходимые библиотеки
target_link_libraries(Client Boost::system Threads::Threads NetCommon)