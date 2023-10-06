cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(LinuxCommandLineChat)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

set(server_src 
    Server/main.cpp 
    Server/Server.cpp 
    Server/Connection.cpp
    Database/UserData.cpp
    Database/UserDatabase.cpp 
    Networking/TCPSocket.cpp)

find_package(SQLite3 REQUIRED)
add_executable(server ${server_src})
target_link_libraries(server PRIVATE SQLite::SQLite3)
