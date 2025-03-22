#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <vector>
#include <chrono>

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

class Logger {
public:
    static void initialize(const std::string& logFilePath = "yolo_fps_assist.log");
    static void setLogLevel(LogLevel level);

    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);
    static void critical(const std::string& message);

    static std::vector<std::string> getRecentLogs(int count = 10);
    static void shutdown();

private:
    static void log(LogLevel level, const std::string& message);
    static std::string getCurrentTime();
    static std::string logLevelToString(LogLevel level);

    static LogLevel s_logLevel;
    static std::string s_logFilePath;
    static std::ofstream s_logFile;
    static std::mutex s_logMutex;
    static std::vector<std::string> s_recentLogs;
    static bool s_initialized;
};