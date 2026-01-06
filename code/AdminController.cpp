#include "AdminController.hpp"
#include "UserService.hpp"
#include "SessionManager.hpp"
#include "AuthService.hpp"
#include "SimpleLogService.hpp"

void mount_admin_routes(crow::SimpleApp& app) {
    using namespace crow;

    // === Lista użytkowników ===
    CROW_ROUTE(app, "/admin/users").methods("GET"_method)([](const request& req) {
        if (!SessionManager::is_valid_session(req) || !SessionManager::has_role(req, "admin"))
            return SessionManager::deny();

        auto users = UserService::loadUsers();
        nlohmann::json list = nlohmann::json::array();

        for (const auto& user : users) {
            list.push_back({
                {"email", user["email"]},
                {"role", user["role"]}
                });
        }

        return response(200, list.dump(4));
        });

    // === Dodawanie użytkownika ===
    CROW_ROUTE(app, "/admin/add_user").methods("POST"_method)([](const request& req) {
        if (!SessionManager::is_valid_session(req) || !SessionManager::has_role(req, "admin"))
            return SessionManager::deny();

        nlohmann::json data;
        try {
            data = nlohmann::json::parse(req.body);
        }
        catch (...) {
            return response(400, "Invalid JSON");
        }

        std::string email = data.value("email", "");
        std::string password = data.value("password", "");
        std::string role = data.value("role", "user");
        std::string adminPass = data.value("admin_password", "");
        std::string adminEmail = SessionManager::get_email_from_cookie(req);

        std::string checkRole;
        if (!AuthService::verify_credentials(adminEmail, adminPass, checkRole) || checkRole != "admin") {
            s_logger.log("[ADMIN] Add user FAILED - invalid admin password by: " + adminEmail);
            return response(403, "Invalid admin password");
        }

        if (email.empty() || password.empty()) {
            return response(400, "Missing required fields");
        }

        s_logger.log("[ADMIN] Add user attempt by: " + adminEmail + ", target = " + email + ", role = " + role);

        if (UserService::addUser(email, password, role)) {
            s_logger.log("[ADMIN] User added: " + email + " (role: " + role + ")");
            return response(200, "User added");
        }
        else {
            s_logger.log("[ADMIN] Add user FAILED - already exists: " + email);
            return response(409, "User already exists");
        }
        });

    // === Usuwanie użytkownika ===
    CROW_ROUTE(app, "/admin/delete_user").methods("POST"_method)([](const request& req) {
        if (!SessionManager::is_valid_session(req) || !SessionManager::has_role(req, "admin"))
            return SessionManager::deny();

        nlohmann::json data;
        try {
            data = nlohmann::json::parse(req.body);
        }
        catch (...) {
            return response(400, "Invalid JSON");
        }

        std::string email = data.value("email", "");
        std::string adminPass = data.value("admin_password", "");
        std::string adminEmail = SessionManager::get_email_from_cookie(req);

        std::string checkRole;
        if (!AuthService::verify_credentials(adminEmail, adminPass, checkRole) || checkRole != "admin") {
            s_logger.log("[ADMIN] Delete user FAILED - invalid admin password by: " + adminEmail);
            return response(403, "Invalid admin password");
        }

        if (email.empty()) {
            return response(400, "Email is required");
        }

        s_logger.log("[ADMIN] Delete user attempt by: " + adminEmail + ", target = " + email);

        if (UserService::deleteUser(email)) {
            s_logger.log("[ADMIN] User deleted: " + email);
            return response(200, "User deleted");
        }
        else {
            s_logger.log("[ADMIN] Delete user FAILED - not found: " + email);
            return response(404, "User not found");
        }
        });
}
