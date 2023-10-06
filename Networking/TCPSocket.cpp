#include "TCPSocket.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>

#include <iostream>
#include <stdexcept>

/*
    SockAddr class implementation
*/

SockAddr::SockAddr() : m_addr{} {
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = 0;
    m_addr.sin_addr.s_addr = INADDR_ANY;
}

SockAddr::SockAddr(const std::string& ip, uint16_t port) {
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &m_addr.sin_addr) != 1) {
        throw std::runtime_error("Failed to convert IP address");
    }
}

SockAddr::SockAddr(sockaddr_in addr) : m_addr{addr} {}

std::string SockAddr::getIp() const {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &m_addr.sin_addr, ip, INET_ADDRSTRLEN);
    return std::string(ip);
}

uint16_t SockAddr::getPort() const {
    return ntohs(m_addr.sin_port);
}

SockAddr::operator std::string() const {
    return getIp() + ":" + std::to_string(getPort());
}

std::ostream& operator<<(std::ostream& os, const SockAddr& addr) {
    os << addr.getIp() << ":" << addr.getPort();
    return os;
}

/*
    TCPSocket class implementation
*/

TCPSocket::TCPSocket() : m_sockfd{-1}, m_localAddr{}, m_remoteAddr{}, m_dataAvailable{false} {}

TCPSocket::TCPSocket(TCPSocketType type) : TCPSocket{} {
    m_sockfd = socket(AF_INET, static_cast<int>(type), 0);
    if (m_sockfd == -1) {
        throw std::runtime_error("Failed to create socket");
    }
}

TCPSocket::TCPSocket(TCPSocket&& other) : m_sockfd{other.m_sockfd}, m_localAddr{other.m_localAddr}, m_remoteAddr{other.m_remoteAddr}, m_dataAvailable{other.m_dataAvailable} {
    other.m_sockfd = -1;
}

TCPSocket::~TCPSocket() {
    // Only close sockets that are not stdin, stdout or stderr
    if (m_sockfd > 2)
    {
        close(m_sockfd);
    }
}

TCPSocket& TCPSocket::operator=(TCPSocket&& other) {
    m_sockfd = other.m_sockfd;
    m_localAddr = other.m_localAddr;
    m_remoteAddr = other.m_remoteAddr;
    m_dataAvailable = other.m_dataAvailable;
    other.m_sockfd = -1;
    return *this;
}

bool TCPSocket::operator==(const TCPSocket& other) const {
    return m_sockfd == other.m_sockfd;
}


bool TCPSocket::connect(const std::string& ip, uint16_t port) {
    sockaddr_in connectAddr;
    connectAddr.sin_family = AF_INET;
    connectAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &connectAddr.sin_addr) != 1) {
        return false;
    }
    if (::connect(m_sockfd, reinterpret_cast<sockaddr*>(&connectAddr), sizeof(connectAddr)) != 0) {
        return false;
    }

    sockaddr_in localAddr;
    socklen_t localAddrLen = sizeof(localAddr);
    ::getsockname(m_sockfd, reinterpret_cast<sockaddr*>(&localAddr), &localAddrLen);
    m_localAddr = SockAddr(localAddr);

    sockaddr_in remoteAddr;
    socklen_t remoteAddrLen = sizeof(remoteAddr);
    ::getpeername(m_sockfd, reinterpret_cast<sockaddr*>(&remoteAddr), &remoteAddrLen);
    m_remoteAddr = SockAddr(remoteAddr);

    return true;
}

bool TCPSocket::bind(uint16_t port) {
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    int ret = ::bind(m_sockfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0;
    if (ret == -1) {
        return false;
    }
    m_localAddr = SockAddr(addr);
    return true;
}

bool TCPSocket::listen(int backlog) {
    return ::listen(m_sockfd, backlog) == 0;
}

TCPSocket TCPSocket::accept() {
    sockaddr_in remoteAddr;
    socklen_t remoteAddrLen = sizeof(remoteAddr);
    int newSockFd = ::accept(m_sockfd, reinterpret_cast<sockaddr*>(&remoteAddr), &remoteAddrLen);
    if (newSockFd == -1) {
        throw std::runtime_error("Failed to accept connection, errno: " + std::to_string(errno));
    }
    TCPSocket newTCPSocket;
    newTCPSocket.m_setSockFd(newSockFd);
    newTCPSocket.m_setRemoteAddr(SockAddr(remoteAddr));
    return newTCPSocket;
}

bool TCPSocket::send(const std::string& data) {
    return ::send(m_sockfd, data.c_str(), data.size(), 0) == static_cast<int>(data.size());
}

std::string TCPSocket::recv(int size) {
    std::vector<char> buffer(size);
    int bytesReceived = ::recv(m_sockfd, buffer.data(), size, 0);
    if (bytesReceived == -1) {
        std::cout << "Failed to receive data, error number: " << errno << std::endl;
        return std::string();
    }
    if (bytesReceived == 0) {
        return std::string();
    }
    return std::string(buffer.data(), bytesReceived);
}

int TCPSocket::getSockFd() const {
    return m_sockfd;
}

const SockAddr& TCPSocket::getLocalAddr() const {
    return m_localAddr;
}

const SockAddr& TCPSocket::getRemoteAddr() const {
    return m_remoteAddr;
}

bool TCPSocket::dataAvailable() {
    m_updateDataAvailable();
    return m_dataAvailable;
}

void TCPSocket::m_updateDataAvailable() {
    pollfd pollFd{m_sockfd, POLLIN, 0};
    ::poll(&pollFd, 1, 0);
    m_dataAvailable = pollFd.revents & POLLIN;
}

void TCPSocket::m_setLocalAddr(SockAddr newAddr) {
    m_localAddr = newAddr;
}

void TCPSocket::m_setRemoteAddr(SockAddr newAddr) {
    m_remoteAddr = newAddr;
}

void TCPSocket::m_setSockFd(int newSockFd) {
    m_sockfd = newSockFd;
}


TCPSocket TCPSocket::stdinSocket() {
    TCPSocket socket(TCPSocketType::TCP);
    socket.m_setSockFd(STDIN_FILENO);
    return socket;
}
