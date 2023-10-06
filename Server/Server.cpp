
#include "Server.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>

Server::Server(uint16_t port, int listenBufferSize) : m_running{false},
                                                      m_port{port},
                                                      m_listenBufferSize{listenBufferSize},
                                                      m_listeningTCPSocket(TCPSocket(TCPSocketType::TCP)),
                                                      m_stdinTCPSocket(TCPSocket::stdinSocket()),
                                                      m_userDatabase{m_databasePath} {}

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
        connection.getSocket().send("Welcome to the server!\nRegister as new user using '/register <name> <password>'\nor login to an existing account using '/login <name> <password>'\n");
        m_newConnections.push_back(std::move(connection));
    }
}

void Server::handleLogin() {
    for (int conn = m_newConnections.size() - 1; conn >= 0; conn--) {
        if (!m_newConnections[conn].dataAvailable()) {
            continue;
        }

        std::string data = m_newConnections[conn].recv();
        if (data.empty()) {
            std::cout << "Connection closed by client: " << m_newConnections[conn].getRemoteAddr() << std::endl;
            m_newConnections.erase(m_newConnections.begin() + conn);
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
            std::cerr << "Client on " << m_newConnections[conn].getRemoteAddr() << " attempts to register, using credentials " << name << ":" << password << std::endl;
            if (name.empty() || password.empty()) {
                m_newConnections[conn].getSocket().send("Invalid name or password\n");
                continue;
            }
            if (name.length() < m_miminumNameLength || name.length() > m_maximumNameLength) {
                m_newConnections[conn].getSocket().send("Name must be between " + std::to_string(m_miminumNameLength) + " and " + std::to_string(m_maximumNameLength) + " characters\n");
                continue;
            }
            if (m_userDatabase.findByName(name).getName() == name) {
                m_newConnections[conn].getSocket().send("Name already taken\n");
                continue;
            }
            if (password.length() < m_miminumNameLength || password.length() > m_maximumNameLength) {
                m_newConnections[conn].getSocket().send("Password must be between " + std::to_string(m_miminumNameLength) + " and " + std::to_string(m_maximumNameLength) + " characters\n");
                continue;
            }

            m_userDatabase.insert(UserData(m_nextClientId++, name, password));

        } else if (command == "/login") {
            std::cerr << "Client on " << m_newConnections[conn].getRemoteAddr() << " attempts to login, using credentials " << name << ":" << password << std::endl;
            if (name.empty() || password.empty()) {
                m_newConnections[conn].getSocket().send("Invalid name or password\n");
                continue;
            }
            if (name.length() < m_miminumNameLength || name.length() > m_maximumNameLength) {
                m_newConnections[conn].getSocket().send("Name must be between " + std::to_string(m_miminumNameLength) + " and " + std::to_string(m_maximumNameLength) + " characters\n");
                continue;
            }
            if (password.length() < m_miminumNameLength || password.length() > m_maximumNameLength) {
                m_newConnections[conn].getSocket().send("Password must be between " + std::to_string(m_miminumNameLength) + " and " + std::to_string(m_maximumNameLength) + " characters\n");
                continue;
            }

            UserData userData = m_userDatabase.findByName(name);
            std::cerr << "Found user: " << userData << std::endl;
            if (userData == UserData::empty() || userData.getPassword() != password) {
                m_newConnections[conn].getSocket().send("Invalid name or password\n");
                continue;
            }

            m_newConnections[conn].setClientData(userData);
            m_approvedConnections.push_back(std::move(m_newConnections[conn]));
            m_newConnections.erase(m_newConnections.begin() + conn);
            sendServerNotification(userData.getName() + " joined the server");
            continue;
        } else {
            m_newConnections[conn].getSocket().send("Invalid command\n");
            continue;
        }
    }
}

void Server::handleApprovedConnections() {
    for (int conn = m_approvedConnections.size() - 1; conn >= 0; conn--) {
        if (m_approvedConnections[conn].dataAvailable()) {
            std::string message = m_approvedConnections[conn].recv();
            if (message.empty()) {
                sendServerNotification(m_approvedConnections[conn].getClientData().getName() + " left the server");
                m_approvedConnections.erase(m_approvedConnections.begin() + conn);
                continue;
            }
            if (message.back() != '\n') {
                message += '\n';
            }

            for (int conn2 = m_approvedConnections.size() - 1; conn2 >= 0; conn2--) {
                if (conn == conn2) {
                    continue;
                }
                m_approvedConnections[conn2].send(m_approvedConnections[conn].getClientData().getName() + ": " + message);
            }

            std::cout << m_approvedConnections[conn].getClientData().getName() << ": " << message;
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
