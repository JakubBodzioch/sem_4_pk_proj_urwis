#include "AutoResponder.hpp"
#include "TicketService.hpp"
#include "SimpleLogService.hpp"

#include <filesystem>
#include <fstream>
#include <unordered_set>
#include <sstream>
#include "json.hpp"
#include <algorithm>
#include <ranges>
#include <thread>

using json = nlohmann::json;
namespace fs = std::filesystem;

AutoResponder s_autoresponder;

bool AutoResponder::load_rules() {
    const std::string path = "stuff/auto_response_ruleset.json";

    if (!fs::exists(path)) {
        s_logger.log("[WIZARD] ruleset not found, generating default: " + path);
        try {
            fs::create_directories(fs::path(path).parent_path());

            json example = json::array({
                {
                    {"keywords", {"printer", "computer"}},
                    {"response", "EXAMPLE: Have you tried to restart the printer and check your Internet access?"}
                }
                });

            std::ofstream out(path);
            if (!out.is_open()) {
                s_logger.log("[WIZARD] Failed to open file for writing: " + path);
                return false;
            }

            out << example.dump(4);
            out.close();

            s_logger.log("[WIZARD] default ruleset created.");
        }
        catch (const std::exception& e) {
            s_logger.log("[WIZARD] error creating ruleset: " + std::string(e.what()));
            return false;
        }
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        s_logger.log("[WIZARD] cannot open ruleset: " + path);
        return false;
    }

    json data;
    try {
        file >> data;
    }
    catch (const std::exception& e) {
        s_logger.log("[WIZARD] error parsing ruleset JSON: " + std::string(e.what()));
        return false;
    }

    std::lock_guard lock(rulesMutex);
    rules.clear();
    for (const auto& item : data) {
        rules.push_back(Rule{
            item["keywords"].get<std::vector<std::string>>(),
            item["response"].get<std::string>()
            });
    }

    s_logger.log("[WIZARD] loaded " + std::to_string(rules.size()) + " autoresponse rules.");
    return true;
}



std::vector<std::string> AutoResponder::extract_keywords(const std::string& text) {
    std::unordered_set<std::string> seen;
    std::istringstream iss(text);
    std::vector<std::string> result;

    for (std::string word; iss >> word;) {
        word = to_lower(word);
        if (seen.insert(word).second) {
            result.push_back(word);
        }
    }
    return result;
}

std::string AutoResponder::to_lower(const std::string& str) {
    std::string lower = str;
    std::ranges::transform(lower, lower.begin(), ::tolower);
    return lower;
}

void AutoResponder::check_and_respond(const std::string& ticketId, const std::string& messageText) {
    auto keywords = extract_keywords(messageText);
    std::lock_guard lock(rulesMutex);

    for (const auto& rule : rules) {
        bool match = std::ranges::all_of(rule.keywords, [&](const std::string& kw) {
            return std::ranges::find(keywords, to_lower(kw)) != keywords.end();
            });

        if (match) {
            s_logger.log("[WIZARD] autoresponse match for ticket: " + ticketId);
            TicketService::replyToTicket(ticketId, rule.response, "wizard@urwis.com");
            break;
        }
    }
}
