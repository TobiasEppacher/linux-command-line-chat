#pragma once

#include <string>

#include "TCPSocket.hpp"

class ClientData {
   private:
    unsigned int m_id;
    bool m_registered;
    std::string m_name;

   public:
    ClientData(unsigned int id);
    ClientData(unsigned int id, const std::string& name);

    unsigned int getId() const;
    bool isRegistered() const;
    const std::string& getName() const;

    void registerClientData(const std::string& name);

    bool operator==(const ClientData& other) const;
};