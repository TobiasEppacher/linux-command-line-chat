#include "Server.hpp"

#include <iostream>

#include "ClientData.hpp"
#include "TCPSocket.hpp"

Server::Server(uint16_t port, int listenBufferSize) : m_running{false},
                                                      m_port{port},
                                                      m_listenBufferSize{listenBufferSize},
                                                      m_listeningTCPSocket(TCPSocket(TCPSocketType::TCP)),
                                                      m_stdinTCPSocket(TCPSocket::stdinSocket()) {}

Server::Command Server::m_parseCommand(const std::string& command) {
    if (command == "exit") {
        return Command::STOP;
    }
    return Command::INVALID;
}

void Server::run() {
    if (!m_listeningTCPSocket.bind(m_port)) {
        std::cout << "bind failed, errno: " << std::to_string(errno) << std::endl;
        throw std::runtime_error("Failed to bind socket");
    }

    if (!m_listeningTCPSocket.listen(m_listenBufferSize)) {
        std::cout << "listen failed, errno: " << std::to_string(errno) << std::endl;
        throw std::runtime_error("Failed to listen on socket");
    }

    m_running = true;
    std::cout << "Server started on port " << m_port << std::endl;

    while (m_running) {
        handleNewConnections();
        handleRegistration();
        handleRegisteredClients();
        handleServerInput();
    }
};

void Server::handleNewConnections() {
    if (m_listeningTCPSocket.dataAvailable()) {
        TCPSocket newClient = m_listeningTCPSocket.accept();
        std::cout << "New connection from: " << newClient.getRemoteAddr() << std::endl;
        newClient.send("Welcome to the server!\nPlease enter your name: ");
        m_unregisteredClients.push_back(std::move(newClient));
    }
}

void Server::handleRegistration() {
    for (auto clientIt = m_unregisteredClients.begin(); clientIt != m_unregisteredClients.end(); clientIt++) {
        if (clientIt->dataAvailable()) {
            std::string name = clientIt->recv();

            if (name.empty()) {
                std::cout << "Connection closed by client: " << clientIt->getRemoteAddr() << std::endl;
                m_unregisteredClients.erase(clientIt);
                break;
            }

            if (name.back() == '\n') {
                name.pop_back();
            }

            if (name.length() < m_miminumNameLength || name.length() > m_maximumNameLength) {
                clientIt->send("Invalid name length, please try again: ");
                continue;
            }

            if (name.find_first_of("\n\t\r\f\v") != std::string::npos) {
                clientIt->send("Name cant contain whitespace, please try again: ");
                continue;
            }

            if (m_registeredClients.find(name) != m_registeredClients.end()) {
                clientIt->send("Name already taken, please try again: ");
            }

            clientIt->send("Welcome to the server, " + name + "!\n");
            sendServerNotification(name + " joined the server");
            m_registeredClients.insert({name, std::move(*clientIt)});
            m_unregisteredClients.erase(clientIt);
            break;
        }
    }
}

void Server::handleRegisteredClients() {
    for (auto clientIt = m_registeredClients.begin(); clientIt != m_registeredClients.end(); clientIt++) {
        if (clientIt->second.dataAvailable()) {
            std::string message = clientIt->second.recv();
            if (message.empty()) {
                sendServerNotification(clientIt->first + " left the server");
                m_registeredClients.erase(clientIt);
                break;
            }
            if (message.back() != '\n') {
                message += '\n';
            }

            for (auto clientIt2 = m_registeredClients.begin(); clientIt2 != m_registeredClients.end(); clientIt2++) {
                if (clientIt2 == clientIt) {
                    continue;
                }
                clientIt2->second.send(clientIt->first + ": " + message);
            }
            
            std::cout << clientIt->first << ": " << message;
        }
    }
}

void Server::handleServerInput() {
    if (m_stdinTCPSocket.dataAvailable()) {
        std::string input{};
        std::getline(std::cin, input);

        if (input.front() != '/') {
            sendServerMessage(input);
            return;
        } else {
            handleServerCommand(input.substr(1));
        }
    }
}

void Server::sendGlobalMessage(const std::string& message, TextColor color) {
    std::string formattedMessage = m_colorizeText(message, color);
    if (message.back() != '\n') {
        formattedMessage += '\n';
    }
    for (auto& client : m_registeredClients) {
        if (!client.second.send(formattedMessage)) {
            std::cout << "Failed to send message to client: " << client.second.getRemoteAddr() << std::endl;
        }
    }
}

void Server::sendServerMessage(const std::string& message) {
    sendGlobalMessage("Server: " + message, TextColor::SERVER_MESSAGE);
}

void Server::sendServerNotification(const std::string& message) {
    std::cout << m_colorizeText(message, TextColor::SERVER_NOTIFICATION) << std::endl;
    sendGlobalMessage(">>> " + message, TextColor::SERVER_NOTIFICATION);
}

void Server::sendServerAlert(const std::string& message) {
    sendGlobalMessage(">>> " + message, TextColor::SERVER_ALERT);
}

void Server::handleServerCommand(const std::string& command) {
    switch (m_parseCommand(command)) {
        case Command::INVALID:
            std::cout << ">>> Invalid command" << std::endl;
            break;
        case Command::STOP:
            sendServerAlert("Server shutdown");
            m_running = false;
            break;

        default:
            break;
    }
}

std::string Server::m_colorizeText(const std::string& text, TextColor color) {
    switch (color) {
        case TextColor::SERVER_ALERT:
            return "\033[31m" + text + "\033[0m";
        case TextColor::SERVER_MESSAGE:
            return "\033[32m" + text + "\033[0m";
        case TextColor::SERVER_NOTIFICATION:
            return "\033[36m" + text + "\033[0m";
        default:
            return text;
    }
}
