#include "KeywordController.hpp"
#include "SessionManager.hpp"
#include "SimpleLogService.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

void mount_keyword_routes(crow::SimpleApp& app) {
    using namespace crow;

    CROW_ROUTE(app, "/admin/keywords").methods("GET"_method)([](const request& req) {
        if (!SessionManager::is_valid_session(req) || !SessionManager::has_role(req, "admin"))
            return SessionManager::deny();

        const std::string filepath = "stuff/keywords.json";
        if (!fs::exists(filepath)) {
            std::ofstream out(filepath);
            if (out.is_open()) {
                out << "[]";
                out.close();
                s_logger.log("[KEYWORDS] keywords.json created (was missing)");
            }
            else {
                s_logger.log("[KEYWORDS] Failed to create keywords.json");
                return response(500, "Cannot create keywords file");
            }
        }

        std::ifstream in(filepath);
        if (!in.is_open()) {
            s_logger.log("[KEYWORDS] Failed to open keywords.json for reading");
            return response(500, "Cannot open keywords file");
        }

        std::ostringstream ss;
        ss << in.rdbuf();
        std::string body = ss.str();

        s_logger.log("[KEYWORDS] keywords.json read successfully (" + std::to_string(body.size()) + " bytes)");

        response res;
        res.code = 200;
        res.set_header("Content-Type", "application/json");
        res.body = body;
        return res;
        });

    CROW_ROUTE(app, "/admin/keywords").methods("POST"_method)([](const request& req) {
        if (!SessionManager::is_valid_session(req) || !SessionManager::has_role(req, "admin"))
            return SessionManager::deny();

        const std::string filepath = "stuff/keywords.json";
        std::ofstream out(filepath);
        if (!out.is_open()) {
            s_logger.log("[KEYWORDS] Failed to open keywords.json for writing");
            return response(500, "Cannot write keywords file");
        }

        out << req.body;
        out.close();

        s_logger.log("[KEYWORDS] keywords.json updated via POST");

        return response(200, "Keywords updated");
        });
}
