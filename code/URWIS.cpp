#include "crow.h"
#include "StaticController.hpp"
#include "AutoResponder.hpp"
#include "AuthController.hpp"
#include "TicketController.hpp"
#include "KeywordController.hpp"
#include "AdminController.hpp"
#include "SimpleLogService.hpp"
#include "CryptoService.hpp"

int main() {

    crow::SimpleApp app;
    
    s_logger.start();
    s_logger.log("[SYSTEM] System starting up");

    mount_static_routes(app);   // login.html, admin.html, user.html, css, logout
    s_logger.log("[SYSTEM] Static routes mounted");

    mount_auth_routes(app);     // login, whoami
    s_logger.log("[SYSTEM] auth routes mounted");

    mount_ticket_routes(app);   // submit_ticket, reply_ticket, itd.
    s_logger.log("[SYSTEM] tickets routes mounted");

    mount_keyword_routes(app);  // keywords API
    s_logger.log("[SYSTEM] keyword routes mounted");

    mount_admin_routes(app);    // user manager & other admin stuff
    s_logger.log("[SYSTEM] admin routes mounted");

    if (s_autoresponder.load_rules()) {
        s_logger.log("[WIZARD] Autoresponder rules loaded successfully.");
    }
    else {
        s_logger.log("[WIZARD] Failed to load autoresponder rules.");
    }

    app.port(3059).multithreaded().run();
}
