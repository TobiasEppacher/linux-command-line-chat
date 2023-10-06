#pragma once

#include "../Networking/TCPSocket.hpp"
#include "../Database/UserData.hpp"

class Connection {
   private:
    TCPSocket m_socket;
    UserData m_clientData;

   public:
    Connection(TCPSocket&& socket, UserData clientData);
    Connection(const Connection& other) = delete;
    Connection(Connection&& other);

    Connection& operator=(const Connection& other) = delete;
    Connection& operator=(Connection&& other);

    bool operator==(const Connection& other) const;

    TCPSocket& getSocket();
    
    UserData& getClientData();
    void setClientData(const UserData& clientData);

    std::string getRemoteAddr() const;
    bool send(const std::string& data);
    std::string recv(int size = 1024);

    bool dataAvailable();
};