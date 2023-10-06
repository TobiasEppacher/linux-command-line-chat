#pragma once

#include <iostream>
#include <string>

#include "../Networking/TCPSocket.hpp"

class UserData {
   private:
    unsigned int m_id;
    std::string m_name;
    std::string m_password;

   public:
    UserData(unsigned int id, const std::string& name, const std::string& password);

    unsigned int getId() const;
    const std::string& getName() const;
    const std::string& getPassword() const;

    unsigned int setId(unsigned int id);
    void setName(const std::string& name);
    void setPassword(const std::string& password);

    bool operator==(const UserData& other) const;

    static UserData empty();

    friend std::ostream& operator<<(std::ostream& os, const UserData& userData) {
        os << "UserData(" << userData.m_id << ", " << userData.m_name << ", " << userData.m_password << ")";
        return os;
    }
};