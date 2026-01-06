#include "UserService.hpp"
#include "CryptoService.hpp"
#include "SimpleLogService.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

static const std::string USER_DB_PATH = "stuff/auth_data.json";
static const std::string ENCRYPTION_KEY = "urwis";

json UserService::loadUsers() {
    if (!fs::exists(USER_DB_PATH)) {
        s_logger.log("[USER] loadUsers - file not found");
        return json::array();
    }

    std::ifstream file(USER_DB_PATH);
    if (!file.is_open()) {
        s_logger.log("[USER] loadUsers - failed to open file");
        return json::array();
    }

    json data;
    file >> data;
    s_logger.log("[USER] loadUsers - loaded " + std::to_string(data.size()) + " user(s)");
    return data;
}

bool UserService::saveUsers(const json& users) {
    std::ofstream out(USER_DB_PATH);
    if (!out.is_open()) {
        s_logger.log("[USER] saveUsers - failed to write to file");
        return false;
    }

    out << users.dump(4);
    s_logger.log("[USER] saveUsers - saved " + std::to_string(users.size()) + " user(s)");
    return true;
}

bool UserService::addUser(const std::string& email, const std::string& password, const std::string& role) {
    json users = loadUsers();

    for (const auto& user : users) {
        if (user["email"] == email) {
            s_logger.log("[USER] addUser - user already exists: " + email);
            return false;
        }
    }

    std::string encrypted = CryptoService::xor_encrypt(password, ENCRYPTION_KEY);

    users.push_back({
        {"email", email},
        {"password", encrypted},
        {"role", role}
        });

    bool result = saveUsers(users);
    if (result)
        s_logger.log("[USER] addUser - added user: " + email + ", role: " + role);
    else
        s_logger.log("[USER] addUser - failed to save user: " + email);

    return result;
}

bool UserService::deleteUser(const std::string& email) {
    json users = loadUsers();
    size_t before = users.size();

    users.erase(std::remove_if(users.begin(), users.end(), [&](const json& user) {
        return user["email"] == email;
        }), users.end());

    bool deleted = users.size() < before;
    if (deleted) {
        bool saved = saveUsers(users);
        if (saved)
            s_logger.log("[USER] deleteUser - deleted user: " + email);
        else
            s_logger.log("[USER] deleteUser - failed to save after delete: " + email);
        return saved;
    }

    s_logger.log("[USER] deleteUser - user not found: " + email);
    return false;
}
