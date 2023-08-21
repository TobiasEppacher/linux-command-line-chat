#include "ClientData.hpp"

ClientData::ClientData(unsigned int id) : m_id(id), m_registered(false) {}

ClientData::ClientData(unsigned int id, const std::string& name) : m_id(id), m_registered(true), m_name(name) {}

unsigned int ClientData::getId() const {
    return m_id;
}

bool ClientData::isRegistered() const {
    return m_registered;
}

const std::string& ClientData::getName() const {
    return m_name;
}

void ClientData::registerClientData(const std::string& name) {
    m_registered = true;
    m_name = name;
}