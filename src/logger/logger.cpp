#include "logger/logger.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <sys/stat.h>
#include <filesystem>

namespace fs = std::filesystem;

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() 
    : m_level(LogLevel::INFO)
    , m_maxFileSize(10 * 1024 * 1024)  // 默认 10MB
    , m_maxFiles(5)                     // 默认保留 5 个文件
    , m_currentSize(0) {
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
    m_filename = filename;
    
    // 确保日志目录存在
    fs::path logPath(filename);
    if (logPath.has_parent_path()) {
        fs::create_directories(logPath.parent_path());
    }
    
    m_file = std::make_unique<std::ofstream>(filename, std::ios::app);
    if (!m_file->is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
    } else {
        m_currentSize = getCurrentFileSize();
    }
}

void Logger::setMaxFileSize(size_t maxBytes) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_maxFileSize = maxBytes;
}

void Logger::setMaxFiles(size_t maxFiles) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_maxFiles = maxFiles;
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

std::string Logger::getCurrentDate() {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d");
    return oss.str();
}

size_t Logger::getCurrentFileSize() {
    if (!m_file || !m_file->is_open()) {
        return 0;
    }
    
    try {
        if (fs::exists(m_filename)) {
            return fs::file_size(m_filename);
        }
    } catch (...) {
        // 忽略错误
    }
    return 0;
}

void Logger::checkAndRotate() {
    // 不需要加锁，调用者已经加锁
    if (m_currentSize >= m_maxFileSize) {
        rotateLogFile();
    }
}

void Logger::rotateLogFile() {
    if (!m_file || !m_file->is_open()) {
        return;
    }
    
    // 关闭当前文件
    m_file->close();
    
    // 生成滚动文件名：filename.1, filename.2, ...
    // 删除最旧的文件
    std::string oldestFile = m_filename + "." + std::to_string(m_maxFiles);
    if (fs::exists(oldestFile)) {
        fs::remove(oldestFile);
    }
    
    // 重命名现有的日志文件
    for (size_t i = m_maxFiles - 1; i >= 1; --i) {
        std::string fromFile = m_filename + "." + std::to_string(i);
        std::string toFile = m_filename + "." + std::to_string(i + 1);
        if (fs::exists(fromFile)) {
            fs::rename(fromFile, toFile);
        }
    }
    
    // 重命名当前日志文件
    std::string firstBackup = m_filename + ".1";
    if (fs::exists(m_filename)) {
        fs::rename(m_filename, firstBackup);
    }
    
    // 打开新的日志文件
    m_file = std::make_unique<std::ofstream>(m_filename, std::ios::app);
    if (!m_file->is_open()) {
        std::cerr << "Failed to reopen log file after rotation: " << m_filename << std::endl;
    }
    m_currentSize = 0;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < m_level) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // 检查是否需要滚动日志
    checkAndRotate();
    
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
        m_currentSize += logMsg.length() + 1;  // +1 for newline
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

