#pragma once

#include <sqlite3.h>

#include <string>

#include "UserData.hpp"

class UserDatabase {
   private:
    sqlite3* m_database;

   public:
    UserDatabase(const std::string& path);
    ~UserDatabase();

    bool insert(const UserData& userData);
    bool update(const UserData& userData);
    bool remove(const UserData& userData);
    UserData findById(unsigned int id);
    UserData findByName(const std::string& name);
};