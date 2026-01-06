#include "AuthController.hpp"
#include "AuthService.hpp"
#include "SessionManager.hpp"
#include "SimpleLogService.hpp"

std::string url_decode(const std::string& in) {
    std::string out;
    char ch;
    int ii;
    for (size_t i = 0; i < in.length(); ++i) {
        if (in[i] == '%') {
            sscanf_s(in.substr(i + 1, 2).c_str(), "%x", &ii);
            ch = static_cast<char>(ii);
            out += ch;
            i += 2;
        }
        else if (in[i] == '+') {
            out += ' ';
        }
        else {
            out += in[i];
        }
    }
    return out;
}

std::string get_form_value_regex(const std::string& body, const std::string& key) {
    std::regex pattern(key + "=([^&]*)");
    std::smatch match;
    if (std::regex_search(body, match, pattern)) {
        return url_decode(match[1].str());
    }
    return "";
}

void mount_auth_routes(crow::SimpleApp& app) {
    using namespace crow;

    CROW_ROUTE(app, "/login").methods("POST"_method)([](const request& req) {
        std::string email = get_form_value_regex(req.body, "email");
        std::string password = get_form_value_regex(req.body, "password");

        s_logger.log("[AUTH] Login attempt: email = " + email);

        std::string role;
        if (AuthService::verify_credentials(email, password, role)) {
            std::string target = (role == "admin") ? "/admin" : "/user";

            s_logger.log("[AUTH] Login SUCCESS: email = " + email + ", role = " + role);

            response res(302);
            res.add_header("Location", target);
            res.add_header("Set-Cookie", "urwis_role=" + role + "; Path=/; HttpOnly; Max-Age=3600");
            res.add_header("Set-Cookie", "urwis_email=" + email + "; Path=/; HttpOnly; Max-Age=3600");
            return res;
        }

        s_logger.log("[AUTH] Login FAILED: email = " + email);
        return response(401, "Invalid login");
        });

    CROW_ROUTE(app, "/whoami").methods("GET"_method)([](const request& req) {
        std::string role = SessionManager::get_role_from_cookie(req);
        if (role.empty()) return response(403, "Unauthorized");

        std::string cookie = req.get_header_value("Cookie");
        size_t pos = cookie.find("urwis_email=");
        if (pos == std::string::npos) return response(403, "No email cookie");

        pos += std::string("urwis_email=").length();
        size_t end = cookie.find(";", pos);
        std::string email = cookie.substr(pos, end == std::string::npos ? end : end - pos);

        response res;
        res.set_header("Content-Type", "application/json");
        res.write("{\"email\":\"" + email + "\", \"username\":\"" + email.substr(0, email.find('@')) + "\", \"role\":\"" + role + "\"}");
        return res;
        });
}
