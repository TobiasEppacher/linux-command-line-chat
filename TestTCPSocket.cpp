#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <thread>

#include "TCPSocket.hpp"
#include "doctest/doctest.h"

TEST_CASE("SockAddr") {
    SUBCASE("Default Constructor") {
        SockAddr defaultAddr;
        CHECK(defaultAddr.getIp() == "0.0.0.0");
        CHECK(defaultAddr.getPort() == 0);
    }

    SUBCASE("Constructor with IP and port") {
        SockAddr addr("192.168.0.1", 8080);
        CHECK(addr.getIp() == "192.168.0.1");
        CHECK(addr.getPort() == 8080);
    }

    SUBCASE("Constructor with sockaddr_in") {
        sockaddr_in addrStruct;
        addrStruct.sin_family = AF_INET;
        addrStruct.sin_port = htons(8081);
        inet_pton(AF_INET, "192.168.0.2", &addrStruct.sin_addr);
        SockAddr addr2(addrStruct);
        CHECK(addr2.getIp() == "192.168.0.2");
        CHECK(addr2.getPort() == 8081);
    }
}

TEST_CASE("TCPSocket") {
    TCPSocket socket;
    SUBCASE("Default Constructor") {
        CHECK(socket.getSockFd() == -1);
        CHECK(socket.getLocalAddr().getIp() == "0.0.0.0");
        CHECK(socket.getLocalAddr().getPort() == 0);
        CHECK(socket.getRemoteAddr().getIp() == "0.0.0.0");
        CHECK(socket.getRemoteAddr().getPort() == 0);
        CHECK(socket.dataAvailable() == false);
    }

    SUBCASE("Constructor with TCPSocketType") {
        CHECK_NOTHROW(socket = TCPSocket(TCPSocketType::TCP));
        CHECK(socket.getSockFd() != -1);
        CHECK(socket.getLocalAddr().getIp() == "0.0.0.0");
        CHECK(socket.getLocalAddr().getPort() == 0);
        CHECK(socket.getRemoteAddr().getIp() == "0.0.0.0");
        CHECK(socket.getRemoteAddr().getPort() == 0);
        CHECK(socket.dataAvailable() == false);

        CHECK_NOTHROW(socket = TCPSocket(TCPSocketType::UDP));
        CHECK(socket.getSockFd() != -1);
        CHECK(socket.getLocalAddr().getIp() == "0.0.0.0");
        CHECK(socket.getLocalAddr().getPort() == 0);
        CHECK(socket.getRemoteAddr().getIp() == "0.0.0.0");
        CHECK(socket.getRemoteAddr().getPort() == 0);
        CHECK(socket.dataAvailable() == false);
    }

    SUBCASE("Bind, Listen, Accept and Connect") {
        TCPSocket listeningTCPSocket = TCPSocket(TCPSocketType::TCP);
        TCPSocket client = TCPSocket(TCPSocketType::TCP);
        SockAddr remoteAddr;

        CHECK(listeningTCPSocket.bind(8082) == true);
        CHECK(listeningTCPSocket.getLocalAddr().getPort() == 8082);
        CHECK(listeningTCPSocket.listen(1) == true);
        std::thread thread(
            [&remoteAddr, &listeningTCPSocket, &client] {
                usleep(200000);
                client.connect(listeningTCPSocket.getLocalAddr().getIp(), listeningTCPSocket.getLocalAddr().getPort());
                remoteAddr = client.getLocalAddr();
            });
        TCPSocket acceptedTCPSocket = listeningTCPSocket.accept();
        CHECK(acceptedTCPSocket.getSockFd() != -1);
        CHECK(acceptedTCPSocket.getRemoteAddr().getIp() == remoteAddr.getIp());

        thread.join();
    }

    SUBCASE("Send and Recv") {
        TCPSocket listeningTCPSocket = TCPSocket(TCPSocketType::TCP);
        TCPSocket client = TCPSocket(TCPSocketType::TCP);
        std::string message = "Hello World!";

        CHECK(listeningTCPSocket.bind(8083) == true);
        CHECK(listeningTCPSocket.getLocalAddr().getPort() == 8083);
        CHECK(listeningTCPSocket.listen(1) == true);
        std::thread thread(
            [&listeningTCPSocket, &client, &message] {
                usleep(200000);
                client.connect(listeningTCPSocket.getLocalAddr().getIp(), listeningTCPSocket.getLocalAddr().getPort());
                usleep(200000);
                client.send(message);
            });
        TCPSocket acceptedTCPSocket = listeningTCPSocket.accept();
        CHECK(acceptedTCPSocket.getSockFd() != -1);
        CHECK(acceptedTCPSocket.recv() == message);

        thread.join();
    }

    SUBCASE("Poll") {
        TCPSocket listeningTCPSocket = TCPSocket(TCPSocketType::TCP);
        TCPSocket client1 = TCPSocket(TCPSocketType::TCP);
        TCPSocket client2 = TCPSocket(TCPSocketType::TCP);
        TCPSocket client3 = TCPSocket(TCPSocketType::TCP);
        std::string messageHead = "Test";

        CHECK(listeningTCPSocket.bind(8084) == true);
        CHECK(listeningTCPSocket.listen(3) == true);

        std::thread thread1(
            [&listeningTCPSocket, &messageHead, &client1] {
                usleep(200000);
                client1.connect(listeningTCPSocket.getLocalAddr().getIp(), listeningTCPSocket.getLocalAddr().getPort());
                usleep(200000);
                client1.send(messageHead + "1");
            });

        std::thread thread2(
            [&listeningTCPSocket, &messageHead, &client2] {
                usleep(200000);
                client2.connect(listeningTCPSocket.getLocalAddr().getIp(), listeningTCPSocket.getLocalAddr().getPort());
                usleep(300000);
                client2.send(messageHead + "2");
            });

        std::thread thread3(
            [&listeningTCPSocket, &messageHead, &client3] {
                usleep(200000);
                client3.connect(listeningTCPSocket.getLocalAddr().getIp(), listeningTCPSocket.getLocalAddr().getPort());
                usleep(500000);
                client3.send(messageHead + "3");
            });

        std::vector<TCPSocket> clients{};
        for (int i = 0; i < 3; i++) {
            clients.push_back(listeningTCPSocket.accept());
        }

        int messages = 3;
        while (messages > 0) {
            //TCPPoll(clients.begin(), clients.end());
            for (auto& client : clients) {
                if (client.dataAvailable()) {
                    std::string message = client.recv();
                    if(!(message.back() == '\n')) {
                        message.push_back('\n');
                    }
                    CHECK(message.substr(0, 4) == messageHead);
                    messages--;
                }
            }
        }

        CHECK(messages == 0);

        thread1.join();
        thread2.join();
        thread3.join();
    }
}
