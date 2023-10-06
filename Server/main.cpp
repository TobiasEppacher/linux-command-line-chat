#include <unistd.h>

#include <chrono>
#include <iostream>
#include <thread>

#include "../Networking/TCPSocket.hpp"
#include "Server.hpp"

int main(int, char** argv) {
    uint16_t port = std::atoi(argv[1]);
    Server server{port};
    server.run();
    return 0;
}