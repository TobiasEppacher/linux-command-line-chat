#pragma once

#include <filesystem>
#include <vector>

#include "../Database/UserData.hpp"
#include "../Database/UserDatabase.hpp"
#include "../Networking/TCPSocket.hpp"
#include "Connection.hpp"

class Server {
    enum class ServerCommand {
        INVALID,
        STOP,
        HELP
    };

    enum class TextColor {
        SERVER_MESSAGE,
        SERVER_NOTIFICATION,
        SERVER_ALERT
    };

   private:
    bool m_running;
    uint16_t m_port;
    TCPSocket m_listeningTCPSocket;
    TCPSocket m_stdinTCPSocket;

    std::string m_databasePath = std::filesystem::current_path().string() + "/users.db";
    UserDatabase m_userDatabase;
    unsigned int m_nextClientId;

    std::vector<Connection> m_newConnections;
    std::vector<Connection> m_approvedConnections;

    const int m_listenBufferSize = 5;
    const unsigned int m_miminumNameLength = 3;
    const unsigned int m_maximumNameLength = 16;
    const unsigned int m_minimumPasswordLength = 6;
    const unsigned int m_maximumPasswordLength = 32;

    const std::string m_welcomeMsg =
        "Welcome to the server!\n\
        Register as new user using '/register <name> <password>'\n\
        or login to an existing account using '/login <name> <password>'\n";

    const std::string m_consoleHelpMsg =
        "Available commands:\n\
        /help - display this message\n\
        /exit - stop the server\n";

    ServerCommand m_parseCommand(const std::string& command);
    std::string m_colorizeText(const std::string& text, TextColor color);

   public:
    /*
     * Constructor
     * @param port - port to listen on
     */
    Server(uint16_t port);

    /*
     * Starts the servers main loop
     */
    void run();

   private:
    /*
     * Handles input from the server console
     * Can either be a normal message to all users or a command (commands start with '/')
     */
    void handleServerInput();

    /*
     * Accepts new connections and adds them to the list of unapproved connections (not logged in users)
     */
    void handleNewConnections();

    /*
     * Handles login and registration requests
     */
    void handleLogin();

    /*
     * Handles approved connections (logged in users), by forwarding messages to the other users
     */
    void handleApprovedConnections();

    /*
     * Handles a command from the server console
     * @param command - command to handle
     */
    void handleServerCommand(const std::string& command);

    /*
     * Sends a message with the selected color to all logged in users
     * This method is a generalization of sendServerMessage, sendServerNotification and sendServerAlert which themselves use predefined colors
     * @param message - message to send
     * @param color - color to use for the message
     */
    void sendGlobalMessage(const std::string& message, TextColor color);

    /*
     * Sends a text message to all logged in users
     */
    void sendServerMessage(const std::string& message);

    /*
     * Sends a notification message to all logged in users
     */
    void sendServerNotification(const std::string& message);

    /*
     * Sends an alert message to all logged in users
     
     */
    void sendServerAlert(const std::string& message);
};