#pragma once
#include <string>
#include <fstream>
#include <filesystem>
#include "json.hpp"

class UserService {
public:
    static bool addUser(const std::string& email, const std::string& password, const std::string& role);
    static bool deleteUser(const std::string& email);
    static nlohmann::json loadUsers();
    static bool saveUsers(const nlohmann::json& users);
};
