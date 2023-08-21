cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(cmake_test VERSION 0.0.1 LANGUAGES CXX)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

# Prepare doctest for other targets to use
find_package(doctest REQUIRED)

# Make test executable
add_executable(tests TestTCPSocket.cpp TCPSocket.cpp)
target_compile_features(tests PRIVATE cxx_std_17)
target_link_libraries(tests PRIVATE doctest::doctest)

add_executable(main main.cpp TCPSocket.cpp Server.cpp ClientData.cpp)
