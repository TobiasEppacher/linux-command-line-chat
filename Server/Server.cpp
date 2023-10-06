
#include "Server.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>

Server::Server(uint16_t port) : m_running{false},
                                m_port{port},
                                m_listeningTCPSocket(TCPSocket(TCPSocketType::TCP)),
                                m_stdinTCPSocket(TCPSocket::stdinSocket()),
                                m_userDatabase{m_databasePath} {}

Server::ServerCommand Server::m_parseCommand(const std::string& command) {
    if (command == "exit") {
        return ServerCommand::STOP;
    }
    if (command == "help") {
        return ServerCommand::HELP;
    }
    return ServerCommand::INVALID;
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

void Server::run() {
    if (!m_listeningTCPSocket.bind(m_port)) {
        std::cerr << "bind failed, errno: " << std::to_string(errno) << std::endl;
        exit(1);
    }

    if (!m_listeningTCPSocket.listen(m_listenBufferSize)) {
        std::cerr << "listen failed, errno: " << std::to_string(errno) << std::endl;
        exit(1);
    }

    m_running = true;
    std::cout << "Server running on port " << m_port << std::endl;

    while (m_running) {
        handleNewConnections();
        handleLogin();
        handleApprovedConnections();
        handleServerInput();
    }
};

void Server::handleNewConnections() {
    if (m_listeningTCPSocket.dataAvailable()) {
        TCPSocket socket = m_listeningTCPSocket.accept();
        Connection connection = Connection(std::move(socket), UserData::empty());
        std::cout << "New connection from: " << connection.getSocket().getRemoteAddr() << std::endl;
        connection.getSocket().send(m_welcomeMsg);
        m_newConnections.push_back(std::move(connection));
    }
}

void Server::handleLogin() {
    for (int connIdx = m_newConnections.size() - 1; connIdx >= 0; connIdx--) {
        if (!m_newConnections[connIdx].dataAvailable()) {
            continue;
        }

        std::string data = m_newConnections[connIdx].recv();
        if (data.empty()) {
            std::cout << "Connection closed by client: " << m_newConnections[connIdx].getRemoteAddr() << std::endl;
            m_newConnections.erase(m_newConnections.begin() + connIdx);
            continue;
        }

        std::istringstream dataStream(data);
        std::string command;
        std::getline(dataStream, command, ' ');
        std::string name;
        std::getline(dataStream, name, ' ');
        std::string password;
        std::getline(dataStream, password);

        if (command == "/register") {
            std::cerr << "Client on " << m_newConnections[connIdx].getRemoteAddr() << " attempts to register, using credentials " << name << ":" << password << std::endl;
            if (name.empty() || password.empty()) {
                m_newConnections[connIdx].getSocket().send("Invalid name or password\n");
                continue;
            }
            if (name.length() < m_miminumNameLength || name.length() > m_maximumNameLength) {
                m_newConnections[connIdx].getSocket().send("Name must be between " + std::to_string(m_miminumNameLength) + " and " + std::to_string(m_maximumNameLength) + " characters\n");
                continue;
            }
            if (m_userDatabase.findByName(name).getName() == name) {
                m_newConnections[connIdx].getSocket().send("Name already taken\n");
                continue;
            }
            if (password.length() < m_minimumPasswordLength || password.length() > m_maximumPasswordLength) {
                m_newConnections[connIdx].getSocket().send("Password must be between " + std::to_string(m_miminumNameLength) + " and " + std::to_string(m_maximumNameLength) + " characters\n");
                continue;
            }

            m_userDatabase.insert(UserData(m_nextClientId++, name, password));

        } else if (command == "/login") {
            std::cerr << "Client on " << m_newConnections[connIdx].getRemoteAddr() << " attempts to login, using credentials " << name << ":" << password << std::endl;
            if (name.empty() || password.empty()) {
                m_newConnections[connIdx].getSocket().send("Invalid name or password\n");
                continue;
            }
            if (name.length() < m_miminumNameLength || name.length() > m_maximumNameLength) {
                m_newConnections[connIdx].getSocket().send("Name must be between " + std::to_string(m_miminumNameLength) + " and " + std::to_string(m_maximumNameLength) + " characters\n");
                continue;
            }
            if (password.length() < m_miminumNameLength || password.length() > m_maximumNameLength) {
                m_newConnections[connIdx].getSocket().send("Password must be between " + std::to_string(m_miminumNameLength) + " and " + std::to_string(m_maximumNameLength) + " characters\n");
                continue;
            }

            UserData userData = m_userDatabase.findByName(name);
            if (userData == UserData::empty() || userData.getPassword() != password) {
                m_newConnections[connIdx].getSocket().send("Invalid name or password\n");
                continue;
            }

            m_newConnections[connIdx].setClientData(userData);
            m_approvedConnections.push_back(std::move(m_newConnections[connIdx]));
            m_newConnections.erase(m_newConnections.begin() + connIdx);
            sendServerNotification(userData.getName() + " joined the server");
            continue;
        } else {
            m_newConnections[connIdx].getSocket().send("Invalid command\n");
            continue;
        }
    }
}

void Server::handleApprovedConnections() {
    for (int connIdx = m_approvedConnections.size() - 1; connIdx >= 0; connIdx--) {
        if (m_approvedConnections[connIdx].dataAvailable()) {
            std::string message = m_approvedConnections[connIdx].recv();
            if (message.empty()) {
                sendServerNotification(m_approvedConnections[connIdx].getClientData().getName() + " left the server");
                m_approvedConnections.erase(m_approvedConnections.begin() + connIdx);
                continue;
            }
            if (message.back() != '\n') {
                message += '\n';
            }

            for (int connIdx2 = m_approvedConnections.size() - 1; connIdx2 >= 0; connIdx2--) {
                if (connIdx == connIdx2) {
                    continue;
                }
                m_approvedConnections[connIdx2].send(m_approvedConnections[connIdx].getClientData().getName() + ": " + message);
            }

            std::cout << m_approvedConnections[connIdx].getClientData().getName() << ": " << message;
        }
    }
}

void Server::handleServerInput() {
    if (m_stdinTCPSocket.dataAvailable()) {
        std::string input{};
        std::getline(std::cin, input);

        if (input.front() == '/') {
            handleServerCommand(input.substr(1));
            return;
        } else {
            sendServerMessage(input);
        }
    }
}

void Server::handleServerCommand(const std::string& command) {
    switch (m_parseCommand(command)) {
        case ServerCommand::INVALID:
            std::cout << ">>> Invalid command" << std::endl;
            break;
        case ServerCommand::STOP:
            sendServerAlert(">>> Server shutdown");
            m_running = false;
            break;
        case ServerCommand::HELP:
            std::cout << m_consoleHelpMsg << std::endl;
        default:
            break;
    }
}

void Server::sendGlobalMessage(const std::string& message, TextColor color) {
    std::string formattedMessage = m_colorizeText(message, color);
    if (message.back() != '\n') {
        formattedMessage += '\n';
    }
    for (auto& conn : m_approvedConnections) {
        if (!conn.send(formattedMessage)) {
            std::cout << "Failed to send message to client: " << conn.getRemoteAddr() << std::endl;
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
