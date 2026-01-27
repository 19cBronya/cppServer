#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <sqlite3.h>

// 对话记录结构
struct Message {
    int id;
    std::string sessionId;
    std::string role;        // "user" 或 "assistant"
    std::string content;
    std::string timestamp;
};

// 会话结构
struct Session {
    std::string sessionId;
    std::string createdAt;
    std::string updatedAt;
    int messageCount;
};

// 数据库管理类
class Database {
public:
    static Database& getInstance();
    
    // 初始化数据库
    bool initialize(const std::string& dbPath);
    
    // 关闭数据库
    void close();
    
    // 会话操作
    bool createSession(const std::string& sessionId);
    bool sessionExists(const std::string& sessionId);
    std::vector<Session> getAllSessions();
    Session getSession(const std::string& sessionId);
    bool deleteSession(const std::string& sessionId);
    
    // 消息操作
    bool addMessage(const std::string& sessionId, const std::string& role, const std::string& content);
    std::vector<Message> getMessages(const std::string& sessionId, int limit = 100);
    std::vector<Message> getRecentMessages(const std::string& sessionId, int count = 10);
    int getMessageCount(const std::string& sessionId);
    
    // 统计
    int getTotalSessions();
    int getTotalMessages();

private:
    Database();
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    bool createTables();
    std::string getCurrentTimestamp();
    
    sqlite3* m_db;
    std::mutex m_mutex;
    bool m_initialized;
};

#endif // DATABASE_H
