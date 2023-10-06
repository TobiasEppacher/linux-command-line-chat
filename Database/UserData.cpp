#include "UserData.hpp"

UserData::UserData(unsigned int id, const std::string& name, const std::string& password) : m_id(id), m_name(name), m_password(password) {}

unsigned int UserData::getId() const {
    return m_id;
}

const std::string& UserData::getName() const {
    return m_name;
}

const std::string& UserData::getPassword() const {
    return m_password;
}

UserData UserData::empty() {
    return UserData(0, "", "");
}

bool UserData::operator==(const UserData& other) const {
    return m_id == other.m_id;
}
