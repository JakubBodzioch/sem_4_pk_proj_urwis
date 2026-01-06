#include "SimpleLogService.hpp"
#include <filesystem>
#include <iostream>

SimpleLogService s_logger;

namespace fs = std::filesystem;

SimpleLogService::SimpleLogService() : running(false) {}

SimpleLogService::~SimpleLogService() {
    stop();
}

void SimpleLogService::start() {
    if (running) return;
    running = true;
    openLogFile();

    worker = std::thread(&SimpleLogService::run, this);
}

void SimpleLogService::stop() {
    if (!running) return;
    {
        std::lock_guard<std::mutex> lock(mtx);
        running = false;
    }
    cv.notify_all();
    if (worker.joinable()) worker.join();
    if (logFile.is_open()) logFile.close();
}

void SimpleLogService::log(const std::string& message) {
    std::time_t now = std::time(nullptr);
    std::tm tm_utc;
#ifdef _WIN32
    gmtime_s(&tm_utc, &now);
#else
    gmtime_r(&now, &tm_utc);
#endif

    std::ostringstream oss;
    oss << "(" << std::put_time(&tm_utc, "%Y-%m-%dT%H:%M:%S") << ") [LOG   * ] " << message << "\n";

    {
        std::lock_guard<std::mutex> lock(mtx);
        messages.push(oss.str());
    }
    cv.notify_one();
}


void SimpleLogService::run() {
    while (running || !messages.empty()) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&]() { return !messages.empty() || !running; });

        while (!messages.empty()) {
            const std::string& msg = messages.front();
            std::cout << msg;
            if (logFile.is_open()) {
                logFile << msg;
                logFile.flush();
            }
            messages.pop();
        }
    }
}

void SimpleLogService::openLogFile() {
    std::time_t now = std::time(nullptr);
    std::tm tm_utc;
#ifdef _WIN32
    gmtime_s(&tm_utc, &now);
#else
    gmtime_r(&now, &tm_utc);
#endif

    std::ostringstream filename;
    filename << "log_" << std::put_time(&tm_utc, "%Y-%m-%dT%H-%M-%S") << ".log";

    fs::create_directories("stuff/logs");
    fs::path logPath = fs::path("stuff/logs") / filename.str();
    logFile.open(logPath);
}

