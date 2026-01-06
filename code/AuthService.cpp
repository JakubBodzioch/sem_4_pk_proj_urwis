#include "AuthService.hpp"
#include "CryptoService.hpp"
#include "UserService.hpp"

bool AuthService::verify_credentials(const std::string& email, const std::string& password, std::string& out_role) {

    auto users = UserService::loadUsers();
    for (const auto& user : users) {
        std::string stored_email = user.value("email", "");
        std::string stored_enc = user.value("password", "");
        std::string decrypted = CryptoService::xor_decrypt(stored_enc, "urwis");

        if (stored_email == email && decrypted == password) {
            out_role = user["role"];
            return true;
        }
    }

    return false;
}

