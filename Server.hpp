#pragma once

#include <unordered_map>
#include <vector>

#include "TCPSocket.hpp"

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
    std::vector<TCPSocket> m_unregisteredClients;
    std::unordered_map<std::string, TCPSocket> m_registeredClients;

    unsigned int m_nextClientId = 0;

    const unsigned int m_miminumNameLength = 3;
    const unsigned int m_maximumNameLength = 16;

    Command m_parseCommand(const std::string& command);
    std::string m_colorizeText(const std::string& text, TextColor color);

   public:
    Server(uint16_t port, int listenBufferSize = 5);
    void run();
    void handleServerInput();
    void handleNewConnections();
    void handleRegistration();
    void handleRegisteredClients();
    
    void sendGlobalMessage(const std::string& message, TextColor colorize);
    void sendServerMessage(const std::string& message);
    void sendServerNotification(const std::string& message);
    void sendServerAlert(const std::string& message);
    void handleServerCommand(const std::string& command);

};

namespace std {

}  // namespace std