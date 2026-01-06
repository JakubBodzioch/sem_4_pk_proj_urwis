#include "TicketService.hpp"
#include "SessionManager.hpp"
#include "AutoResponder.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <ctime>
#include <random>
#include <iomanip>

namespace fs = std::filesystem;
using json = nlohmann::json;

std::string TicketService::getCurrentDateISO8601() {
    std::time_t now = std::time(nullptr);
    std::tm tm_utc;
#ifdef _WIN32
    gmtime_s(&tm_utc, &now);
#else
    gmtime_r(&now, &tm_utc);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_utc, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

std::string TicketService::generateTicketId() {
    std::ostringstream oss;
    std::time_t t = std::time(nullptr);
    oss << t << "." << rand() % 10000;
    return oss.str();
}

std::string TicketService::extractUsername(const std::string& email) {
    size_t pos = email.find('@');
    return (pos != std::string::npos) ? email.substr(0, pos) : "unknown";
}

bool TicketService::saveTicket(const std::string& username, const json& ticket) {
    std::string path = "stuff/users/" + username + "/tickets";
    fs::create_directories(path);
    std::ofstream out(path + "/" + ticket["id"].get<std::string>() + ".json");
    if (!out.is_open()) return false;
    out << ticket.dump(4);
    return true;
}

bool TicketService::replyToTicket(const std::string& id, const std::string& msg, const std::string& email) {
    for (const auto& user_dir : fs::directory_iterator("stuff/users")) {
        if (!fs::is_directory(user_dir)) continue;
        std::string path = user_dir.path().string() + "/tickets/" + id + ".json";
        if (!fs::exists(path)) continue;

        std::ifstream in(path);
        if (!in.is_open()) continue;

        json ticket;
        in >> ticket;
        in.close();

        if (ticket["status"] != "open") return false;

        ticket["conversation"].push_back({ {"from", email}, {"msg", msg} });

        std::ofstream out(path);
        if (!out.is_open()) return false;
        out << ticket.dump(4);
        return true;
    }
    return false;
}

bool TicketService::closeTicket(const std::string& id) {
    for (const auto& user_dir : fs::directory_iterator("stuff/users")) {
        if (!fs::is_directory(user_dir)) continue;
        std::string path = user_dir.path().string() + "/tickets/" + id + ".json";
        if (!fs::exists(path)) continue;

        std::ifstream in(path);
        if (!in.is_open()) return false;

        json ticket;
        in >> ticket;
        in.close();

        ticket["status"] = "closed";

        std::ofstream out(path);
        if (!out.is_open()) return false;
        out << ticket.dump(4);
        return true;
    }
    return false;
}

bool TicketService::deleteTicket(const std::string& id) {
    for (const auto& user_dir : fs::directory_iterator("stuff/users")) {
        if (!fs::is_directory(user_dir)) continue;

        std::string userPath = user_dir.path().string();
        std::string ticketPath = userPath + "/tickets/" + id + ".json";
        std::string archiveDir = userPath + "/tickets_old";
        std::string archivePath = archiveDir + "/" + id + ".json";

        if (!fs::exists(ticketPath)) continue;

        try {
            fs::create_directories(archiveDir);

            fs::rename(ticketPath, archivePath);

            return true;
        }
        catch (const std::exception& e) {
            std::cerr << "[TICKET] Error archiving ticket: " << e.what() << std::endl;
            return false;
        }
    }
    return false;
}

json TicketService::getTicketsForUser(const std::string& username) {
    std::string dir = "stuff/users/" + username + "/tickets";
    json all = json::array();
    if (!fs::exists(dir)) return all;

    for (const auto& file : fs::directory_iterator(dir)) {
        std::ifstream in(file.path());
        if (!in.is_open()) continue;
        json ticket;
        in >> ticket;
        all.push_back(ticket);
    }
    return all;
}

json TicketService::getAllTickets() {
    json all_tickets = json::array();
    for (const auto& user_dir : fs::directory_iterator("stuff/users")) {
        if (!fs::is_directory(user_dir)) continue;
        std::string tickets_dir = user_dir.path().string() + "/tickets";
        if (!fs::exists(tickets_dir)) continue;

        for (const auto& file : fs::directory_iterator(tickets_dir)) {
            std::ifstream in(file.path());
            if (!in.is_open()) continue;
            try {
                json ticket;
                in >> ticket;
                ticket["username"] = user_dir.path().filename().string();
                all_tickets.push_back(ticket);
            }
            catch (...) {
                // corrupted file
            }
        }
    }
    return all_tickets;
}
