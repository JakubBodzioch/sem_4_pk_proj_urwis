#pragma once
#include <string>
#include <vector>
#include "json.hpp"
#include "AutoResponder.hpp"

class TicketService 
{
public:
    static std::string generateTicketId();
    static std::string getCurrentDateISO8601();
    static std::string extractUsername(const std::string& email);
    static bool saveTicket(const std::string& username, const nlohmann::json& ticket);
    static bool replyToTicket(const std::string& id, const std::string& msg, const std::string& role);
    static bool closeTicket(const std::string& id);
    static bool deleteTicket(const std::string& id);
    static nlohmann::json getTicketsForUser(const std::string& username);
    static nlohmann::json getAllTickets();
};
