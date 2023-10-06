#include "UserDatabase.hpp"

#include <iostream>

UserDatabase::UserDatabase(const std::string& path) {
    std::cerr << "Opening database at " << path << std::endl;
    int result = sqlite3_open(path.c_str(), &m_database);
    if (result != SQLITE_OK) {
        std::cerr << "Failed to open database: " << sqlite3_errmsg(m_database) << std::endl;
        sqlite3_close(m_database);
        exit(1);
    }
    std::cerr << "Database opened successfully" << std::endl;

    std::cerr << "Creating table users if not exists" << std::endl;
    std::string createTableStmt = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE, password TEXT NOT NULL);";
    char* errMsg;
    result = sqlite3_exec(m_database, createTableStmt.c_str(), nullptr, nullptr, &errMsg);
    if (result != SQLITE_OK) {
        std::cerr << "Failed to create table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        sqlite3_close(m_database);
        exit(1);
    }
    std::cerr << "Table users created successfully" << std::endl;
}

UserDatabase::~UserDatabase() {
    sqlite3_close(m_database);
}

bool UserDatabase::insert(const UserData& userData) {
    std::string insertStmtBase = "INSERT INTO users (id, name, password) VALUES ($id, $name, $password);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_database, insertStmtBase.c_str(), insertStmtBase.length(), &stmt, nullptr);
    sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, "$id"), userData.getId());
    sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, "$name"), userData.getName().c_str(), userData.getName().length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, "$password"), userData.getPassword().c_str(), userData.getPassword().length(), SQLITE_STATIC);
    int result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        std::cerr << "Failed to insert user: " << sqlite3_errmsg(m_database) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

bool UserDatabase::update(const UserData& userData) {
    std::string updateStmtBase = "UPDATE users SET name = $name, password = $password WHERE id = $id;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_database, updateStmtBase.c_str(), updateStmtBase.length(), &stmt, nullptr);
    sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, "$id"), userData.getId());
    sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, "$name"), userData.getName().c_str(), userData.getName().length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, "$password"), userData.getPassword().c_str(), userData.getPassword().length(), SQLITE_STATIC);
    int result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        std::cerr << "Failed to update user: " << sqlite3_errmsg(m_database) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

bool UserDatabase::remove(const UserData& userData) {
    std::string deleteStmtBase = "DELETE FROM users WHERE id = $id;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_database, deleteStmtBase.c_str(), deleteStmtBase.length(), &stmt, nullptr);
    sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, "$id"), userData.getId());
    int result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        std::cerr << "Failed to delete user: " << sqlite3_errmsg(m_database) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

UserData UserDatabase::findById(unsigned int id) {
    std::string selectStmtBase = "SELECT id, name, password FROM users WHERE id = $id;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_database, selectStmtBase.c_str(), selectStmtBase.length(), &stmt, nullptr);
    sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, "$id"), id);
    int result = sqlite3_step(stmt);
    if (result == SQLITE_ROW) {
        unsigned int id = sqlite3_column_int(stmt, 0);
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        sqlite3_finalize(stmt);
        return UserData(id, name, password);
    }
    sqlite3_finalize(stmt);
    return UserData::empty();
}

UserData UserDatabase::findByName(const std::string& name) {
    std::string selectStmtBase = "SELECT id, name, password FROM users WHERE name = $name;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_database, selectStmtBase.c_str(), selectStmtBase.length(), &stmt, nullptr);
    sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, "$name"), name.c_str(), name.length(), SQLITE_STATIC);
    int result = sqlite3_step(stmt);
    if (result == SQLITE_ROW) {
        unsigned int id = sqlite3_column_int(stmt, 0);
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::cerr << "Found user: " << id << ", " << name << ", " << password << std::endl;
        sqlite3_finalize(stmt);
        return UserData(id, name, password);
    }
    sqlite3_finalize(stmt);
    return UserData::empty();
}
