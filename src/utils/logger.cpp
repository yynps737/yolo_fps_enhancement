#include "logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>

LogLevel Logger::s_logLevel = LogLevel::Info;
std::string Logger::s_logFilePath = "yolo_fps_assist.log";
std::ofstream Logger::s_logFile;
std::mutex Logger::s_logMutex;
std::vector<std::string> Logger::s_recentLogs;
bool Logger::s_initialized = false;

void Logger::initialize(const std::string& logFilePath) {
    std::lock_guard<std::mutex> lock(s_logMutex);

    if (s_initialized) {
        return;
    }

    s_logFilePath = logFilePath;
    s_logFile.open(s_logFilePath, std::ios::out | std::ios::app);

    if (!s_logFile.is_open()) {
        std::cerr << "无法打开日志文件：" << s_logFilePath << std::endl;
        return;
    }

    s_initialized = true;
    log(LogLevel::Info, "日志系统初始化");
}

void Logger::setLogLevel(LogLevel level) {
    s_logLevel = level;
}

void Logger::debug(const std::string& message) {
    log(LogLevel::Debug, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::Info, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::Warning, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::Error, message);
}

void Logger::critical(const std::string& message) {
    log(LogLevel::Critical, message);
}

std::vector<std::string> Logger::getRecentLogs(int count) {
    std::lock_guard<std::mutex> lock(s_logMutex);

    if (s_recentLogs.size() <= count) {
        return s_recentLogs;
    }

    return std::vector<std::string>(
        s_recentLogs.end() - count,
        s_recentLogs.end()
    );
}

void Logger::shutdown() {
    std::lock_guard<std::mutex> lock(s_logMutex);

    if (!s_initialized) {
        return;
    }

    log(LogLevel::Info, "日志系统关闭");

    if (s_logFile.is_open()) {
        s_logFile.close();
    }

    s_initialized = false;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < s_logLevel) {
        return;
    }

    std::lock_guard<std::mutex> lock(s_logMutex);

    std::string currentTime = getCurrentTime();
    std::string levelStr = logLevelToString(level);

    std::stringstream ss;
    ss << currentTime << " [" << levelStr << "] " << message;
    std::string logMessage = ss.str();

    std::cout << logMessage << std::endl;

    if (s_initialized && s_logFile.is_open()) {
        s_logFile << logMessage << std::endl;
        s_logFile.flush();
    }

    s_recentLogs.push_back(logMessage);

    while (s_recentLogs.size() > 100) {
        s_recentLogs.erase(s_recentLogs.begin());
    }
}

std::string Logger::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();

    return ss.str();
}

std::string Logger::logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warning:
            return "WARNING";
        case LogLevel::Error:
            return "ERROR";
        case LogLevel::Critical:
            return "CRITICAL";
        default:
            return "UNKNOWN";
    }
}