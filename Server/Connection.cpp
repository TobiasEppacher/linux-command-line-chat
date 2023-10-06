#include "Connection.hpp"

Connection::Connection(TCPSocket&& socket, UserData clientData) : m_socket(std::move(socket)), m_clientData(clientData) {}

Connection::Connection(Connection&& other) : m_socket(std::move(other.m_socket)), m_clientData(other.m_clientData) {}

Connection& Connection::operator=(Connection&& other) {
    m_socket = std::move(other.m_socket);
    m_clientData = other.m_clientData;
    return *this;
}

bool Connection::operator==(const Connection& other) const {
    return m_socket == other.m_socket;
}

TCPSocket& Connection::getSocket() {
    return m_socket;
}

UserData& Connection::getClientData() {
    return m_clientData;
}

void Connection::setClientData(const UserData& clientData) {
    m_clientData = clientData;
}

std::string Connection::getRemoteAddr() const {
    return m_socket.getRemoteAddr();
}

bool Connection::send(const std::string& data) {
    return m_socket.send(data);
}

std::string Connection::recv(int size) {
    return m_socket.recv(size);
}

bool Connection::dataAvailable() {
    return m_socket.dataAvailable();
}