#pragma once

#include <string>
#include <vector>
#include <mutex>

class TicketService;

class AutoResponder {
public:
    bool load_rules();
    void check_and_respond(const std::string& ticketId, const std::string& messageText);

private:
    struct Rule {
        std::vector<std::string> keywords;
        std::string response;
    };

    std::vector<Rule> rules;
    std::mutex rulesMutex;

    std::vector<std::string> extract_keywords(const std::string& text);
    std::string to_lower(const std::string& str);
};

extern AutoResponder s_autoresponder;
