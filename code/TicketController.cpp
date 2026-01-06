#include "TicketController.hpp"
#include "TicketService.hpp"
#include "KeywordService.hpp"
#include "SessionManager.hpp"
#include "SimpleLogService.hpp"

void mount_ticket_routes(crow::SimpleApp& app) {
    using namespace crow;

    CROW_ROUTE(app, "/submit_ticket").methods("POST"_method)([](const request& req) {
        if (!SessionManager::has_role(req, "user")) return SessionManager::deny();

        std::string cookie = req.get_header_value("Cookie");
        size_t pos = cookie.find("urwis_email=");
        if (pos == std::string::npos) return response(403, "No email cookie");
        pos += std::string("urwis_email=").length();
        size_t end = cookie.find(";", pos);
        std::string email = cookie.substr(pos, end == std::string::npos ? end : end - pos);

        std::string username = TicketService::extractUsername(email);
        std::string id = TicketService::generateTicketId();
        std::string message = req.body;

        auto keywords = KeywordService::extract_keywords(message);
        nlohmann::json ticket = {
            {"id", id},
            {"author", email},
            {"keywords", keywords},
            {"status", "open"},
            {"created_at", TicketService::getCurrentDateISO8601()},
            {"conversation", nlohmann::json::array({{{"from", email}, {"msg", message}}})}
        };

        s_logger.log("[TICKET] Creating ticket ID: " + id + " by user: " + email);

        bool ok = TicketService::saveTicket(username, ticket);
        if (ok) {
            s_logger.log("[TICKET] Ticket saved successfully: " + id);

            s_autoresponder.check_and_respond(id, message);

            return response(200, "Ticket created with ID: " + id);
        }
        else {
            s_logger.log("[TICKET] Failed to save ticket: " + id);
            return response(500, "Failed to save ticket");
        }
        });

    CROW_ROUTE(app, "/reply_ticket").methods("POST"_method)([](const request& req) {
        std::string role = SessionManager::get_role_from_cookie(req);
        if (role != "admin" && role != "user") return SessionManager::deny();

        std::string id = req.url_params.get("id");
        std::string email = SessionManager::get_email_from_cookie(req);
        std::string msg = req.body;

        s_logger.log("[TICKET] Reply attempt to ticket ID: " + std::string(id) + " from: " + email);

        bool ok = TicketService::replyToTicket(id, msg, email);
        if (ok) {
            s_logger.log("[TICKET] Reply added to ticket ID: " + std::string(id));
            return response(200, "Reply added");
        }
        else {
            s_logger.log("[TICKET] Failed to add reply to ticket ID: " + std::string(id));
            return response(400, "Failed to add reply");
        }
        });

    CROW_ROUTE(app, "/admin/close_ticket").methods("POST"_method)([](const request& req) {
        if (!SessionManager::is_valid_session(req) || !SessionManager::has_role(req, "admin"))
            return SessionManager::deny();

        std::string id = req.url_params.get("id");
        s_logger.log("[TICKET] Attempt to close ticket ID: " + id);

        bool ok = TicketService::closeTicket(id);
        if (ok) {
            s_logger.log("[TICKET] Ticket closed: " + id);
            return response(200, "Ticket closed");
        }
        else {
            s_logger.log("[TICKET] Failed to close ticket: " + id);
            return response(400, "Failed to close ticket");
        }
        });

    CROW_ROUTE(app, "/my_tickets").methods("GET"_method)([](const request& req) {
        if (!SessionManager::has_role(req, "user")) return SessionManager::deny();

        std::string username = req.url_params.get("user");
        s_logger.log("[TICKET] Fetching tickets for user: " + username);

        auto tickets = TicketService::getTicketsForUser(username);
        response res;
        res.set_header("Content-Type", "application/json");
        res.write(tickets.dump(4));
        return res;
        });

    CROW_ROUTE(app, "/admin/tickets").methods("GET"_method)([](const request& req) {
        if (!SessionManager::is_valid_session(req) || !SessionManager::has_role(req, "admin"))
            return SessionManager::deny();

        s_logger.log("[TICKET] Admin fetching all tickets");

        auto tickets = TicketService::getAllTickets();
        response res;
        res.set_header("Content-Type", "application/json");
        res.write(tickets.dump(4));
        return res;
        });

    CROW_ROUTE(app, "/admin/delete_ticket").methods("POST"_method)([](const request& req) {
        if (!SessionManager::is_valid_session(req) || !SessionManager::has_role(req, "admin"))
            return SessionManager::deny();

        std::string id = req.url_params.get("id");
        s_logger.log("[TICKET] Attempt to delete ticket ID: " + id);

        bool ok = TicketService::deleteTicket(id);
        if (ok) {
            s_logger.log("[TICKET] Ticket deleted: " + id);
            return response(200, "Ticket deleted");
        }
        else {
            s_logger.log("[TICKET] Failed to delete ticket: " + id);
            return response(400, "Failed to delete ticket");
        }
        });
}
