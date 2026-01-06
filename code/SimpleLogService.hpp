#pragma once
#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <fstream>

class SimpleLogService {
public:
    SimpleLogService();
    ~SimpleLogService();

    void start();
    void stop();
    void log(const std::string& message);

private:
    void run();
    void openLogFile();

    std::atomic<bool> running;
    std::thread worker;

    std::queue<std::string> messages;
    std::mutex mtx;
    std::condition_variable cv;

    std::ofstream logFile;
};

extern SimpleLogService s_logger;