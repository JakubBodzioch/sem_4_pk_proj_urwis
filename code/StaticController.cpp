#include "StaticController.hpp"
#include "SessionManager.hpp"
#include "SimpleLogService.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>

crow::response serve_file(const std::string& path, const std::string& content_type) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        s_logger.log("[STATIC] File not found: " + path);
        return crow::response(404);
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();

    s_logger.log("[STATIC] Served file: " + path);

    crow::response res(buffer.str());
    res.add_header("Content-Type", content_type);
    return res;
}

void mount_static_routes(crow::SimpleApp& app) {
    using namespace crow;

    CROW_ROUTE(app, "/")([] {
        return serve_file("public/login.html", "text/html; charset=utf-8");
        });

    CROW_ROUTE(app, "/login")([] {
        return serve_file("public/login.html", "text/html; charset=utf-8");
        });

    CROW_ROUTE(app, "/admin")([](const request& req) {
        if (!SessionManager::is_valid_session(req) || !SessionManager::has_role(req, "admin"))
            return SessionManager::deny();
        return serve_file("public/sites/admin.html", "text/html; charset=utf-8");
        });

    CROW_ROUTE(app, "/user")([](const request& req) {
        if (!SessionManager::has_role(req, "user")) return SessionManager::deny();
        return serve_file("public/sites/user.html", "text/html; charset=utf-8");
        });

    CROW_ROUTE(app, "/logout").methods(crow::HTTPMethod::GET)([]() {
        s_logger.log("[STATIC] Logout route hit, clearing cookies");
        crow::response res;
        res.code = 302;
        res.set_header("Location", "/login");
        res.add_header("Set-Cookie", "urwis_role=deleted; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT");
        res.add_header("Set-Cookie", "urwis_email=deleted; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT");
        return res;
        });

    CROW_ROUTE(app, "/css/<string>")([](const request&, std::string file) {
        return serve_file("public/css/" + file, "text/css; charset=utf-8");
        });

    CROW_ROUTE(app, "/img/<string>")([](const request&, std::string file) {
        return serve_file("public/img/" + file, "image/png");
        });
}
