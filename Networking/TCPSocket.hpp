#pragma once

#include <arpa/inet.h>

#include <string>
#include <vector>

enum class TCPSocketType {
    TCP = SOCK_STREAM,
    UDP = SOCK_DGRAM,
};

class SockAddr {
   private:
    sockaddr_in m_addr;

   public:
    SockAddr();
    SockAddr(const std::string& ip, uint16_t port);
    SockAddr(sockaddr_in addr);
    std::string getIp() const;
    uint16_t getPort() const;

    operator std::string() const;
    friend std::ostream& operator<<(std::ostream& os, const SockAddr& addr);
};

class TCPSocket {
   private:
    int m_sockfd;
    SockAddr m_localAddr;
    SockAddr m_remoteAddr;
    bool m_dataAvailable;

   public:
    TCPSocket();
    TCPSocket(TCPSocketType type);
    TCPSocket(TCPSocket&& other);
    TCPSocket(const TCPSocket& other) = delete;
    ~TCPSocket();

    TCPSocket& operator=(TCPSocket&& other);
    TCPSocket& operator=(const TCPSocket& other) = delete;
    bool operator==(const TCPSocket& other) const;


    bool connect(const std::string& ip, uint16_t port);
    bool bind(uint16_t port);
    bool listen(int backlog);
    TCPSocket accept();
    bool send(const std::string& data);
    std::string recv(int size = 1024);

    int getSockFd() const;
    const SockAddr& getLocalAddr() const;
    const SockAddr& getRemoteAddr() const;
    bool dataAvailable();

    void m_setSockFd(int newSockFd);
    void m_setLocalAddr(SockAddr newAddr);
    void m_setRemoteAddr(SockAddr newAddr);
    void m_updateDataAvailable();

    static TCPSocket stdinSocket();
};