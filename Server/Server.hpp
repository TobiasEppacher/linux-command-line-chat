#pragma once

#include <filesystem>
#include <vector>

#include "../Database/UserData.hpp"
#include "../Database/UserDatabase.hpp"
#include "../Networking/TCPSocket.hpp"
#include "Connection.hpp"

class Server {
    enum class Command {
        INVALID,
        STOP
    };

    enum class TextColor {
        SERVER_MESSAGE,
        SERVER_NOTIFICATION,
        SERVER_ALERT
    };

   private:
    bool m_running;
    uint16_t m_port;
    int m_listenBufferSize;
    TCPSocket m_listeningTCPSocket;
    TCPSocket m_stdinTCPSocket;

    std::string m_databasePath = std::filesystem::current_path().string() + "/users.db";
    UserDatabase m_userDatabase;
    std::vector<Connection> m_newConnections;
    std::vector<Connection> m_approvedConnections;

    unsigned int m_nextClientId = 1;

    const unsigned int m_miminumNameLength = 3;
    const unsigned int m_maximumNameLength = 16;

    Command m_parseCommand(const std::string& command);
    std::string m_colorizeText(const std::string& text, TextColor color);

   public:
    Server(uint16_t port, int listenBufferSize = 5);
    void run();
    void handleServerInput();
    void handleNewConnections();
    void handleLogin();
    void handleApprovedConnections();

    void sendGlobalMessage(const std::string& message, TextColor colorize);
    void sendServerMessage(const std::string& message);
    void sendServerNotification(const std::string& message);
    void sendServerAlert(const std::string& message);
    void handleServerCommand(const std::string& command);
};

namespace std {

}  // namespace std