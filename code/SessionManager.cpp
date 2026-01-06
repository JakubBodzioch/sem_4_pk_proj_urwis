#include "SessionManager.hpp"
#include "UserService.hpp"
#include "SimpleLogService.hpp"

std::string SessionManager::get_role_from_cookie(const crow::request& req) {
    std::string cookie = req.get_header_value("Cookie");
    size_t pos = cookie.find("urwis_role=");
    if (pos == std::string::npos) return "";
    pos += std::string("urwis_role=").length();
    size_t end = cookie.find(";", pos);
    return cookie.substr(pos, end - pos);
}

std::string SessionManager::get_email_from_cookie(const crow::request& req) {
    std::string cookie = req.get_header_value("Cookie");
    size_t pos = cookie.find("urwis_email=");
    if (pos == std::string::npos) return "";
    pos += std::string("urwis_email=").length();
    size_t end = cookie.find(";", pos);
    return cookie.substr(pos, end - pos);
}

bool SessionManager::has_role(const crow::request& req, const std::string& role) {
    std::string currentRole = get_role_from_cookie(req);
    s_logger.log("[SESSION] has_role check: cookie role = " + currentRole + ", required = " + role);
    return currentRole == role;
}

bool SessionManager::is_valid_session(const crow::request& req) {
    std::string email = get_email_from_cookie(req);
    std::string role = get_role_from_cookie(req);

    auto users = UserService::loadUsers();
    for (const auto& user : users) {
        if (user["email"] == email && user["role"] == role) {
            s_logger.log("[SESSION] is_valid_session: email = " + email + ", role = " + role + " → VALID");
            return true;
        }
    }

    s_logger.log("[SESSION] is_valid_session: email = " + email + ", role = " + role + " → INVALID");
    return false;
}

crow::response SessionManager::deny() {
    return crow::response(403, "Access denied");
}
