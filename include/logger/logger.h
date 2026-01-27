#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <memory>
#include <mutex>
#include <cstdint>

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    FATAL = 4
};

class Logger {
public:
    static Logger& getInstance();
    
    void setLevel(LogLevel level);
    void setLogFile(const std::string& filename);
    
    // 设置日志滚动策略
    void setMaxFileSize(size_t maxBytes);  // 按文件大小滚动
    void setMaxFiles(size_t maxFiles);     // 保留的最大日志文件数
    
    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);
    
    void log(LogLevel level, const std::string& message);

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    std::string levelToString(LogLevel level);
    std::string getCurrentTime();
    std::string getCurrentDate();
    
    // 日志滚动相关
    void checkAndRotate();
    void rotateLogFile();
    size_t getCurrentFileSize();
    
    LogLevel m_level;
    std::unique_ptr<std::ofstream> m_file;
    std::mutex m_mutex;
    std::string m_filename;
    size_t m_maxFileSize;  // 最大文件大小（字节）
    size_t m_maxFiles;     // 保留的最大文件数
    size_t m_currentSize;  // 当前文件大小
};

// 便捷宏
#define LOG_DEBUG(msg) Logger::getInstance().debug(msg)
#define LOG_INFO(msg) Logger::getInstance().info(msg)
#define LOG_WARN(msg) Logger::getInstance().warn(msg)
#define LOG_ERROR(msg) Logger::getInstance().error(msg)
#define LOG_FATAL(msg) Logger::getInstance().fatal(msg)

#endif // LOGGER_H

