#include "logger/logger.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : m_level(LogLevel::INFO) {
}

Logger::~Logger() {
    if (m_file && m_file->is_open()) {
        m_file->close();
    }
}

void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_level = level;
}

void Logger::setLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_file = std::make_unique<std::ofstream>(filename, std::ios::app);
    if (!m_file->is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
    }
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

std::string Logger::getCurrentTime() {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < m_level) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::string logMsg = "[" + getCurrentTime() + "] [" + levelToString(level) + "] " + message;
    
    // 输出到控制台
    if (level >= LogLevel::ERROR) {
        std::cerr << logMsg << std::endl;
    } else {
        std::cout << logMsg << std::endl;
    }
    
    // 输出到文件
    if (m_file && m_file->is_open()) {
        *m_file << logMsg << std::endl;
        m_file->flush();
    }
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warn(const std::string& message) {
    log(LogLevel::WARN, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::fatal(const std::string& message) {
    log(LogLevel::FATAL, message);
}

