#pragma once
#include <string>
#include "crow.h"

class SessionManager {
public:
    static std::string get_role_from_cookie(const crow::request& req);
    static std::string get_email_from_cookie(const crow::request& req);
    static bool has_role(const crow::request& req, const std::string& role);
    static crow::response deny();
    static bool is_valid_session(const crow::request& req);
};
