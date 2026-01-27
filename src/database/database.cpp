#include "database/database.h"
#include "logger/logger.h"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <filesystem>

namespace fs = std::filesystem;

Database& Database::getInstance() {
    static Database instance;
    return instance;
}

Database::Database() : m_db(nullptr), m_initialized(false) {
}

Database::~Database() {
    close();
}

bool Database::initialize(const std::string& dbPath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_initialized) {
        LOG_WARN("Database already initialized");
        return true;
    }
    
    // 确保数据库目录存在
    fs::path path(dbPath);
    if (path.has_parent_path()) {
        fs::create_directories(path.parent_path());
    }
    
    // 打开数据库
    int rc = sqlite3_open(dbPath.c_str(), &m_db);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to open database: " + std::string(sqlite3_errmsg(m_db)));
        return false;
    }
    
    LOG_INFO("Database opened: " + dbPath);
    
    // 创建表
    if (!createTables()) {
        LOG_ERROR("Failed to create tables");
        return false;
    }
    
    m_initialized = true;
    LOG_INFO("Database initialized successfully");
    return true;
}

void Database::close() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
        m_initialized = false;
        LOG_INFO("Database closed");
    }
}

bool Database::createTables() {
    // 创建会话表
    const char* createSessionsTable = R"(
        CREATE TABLE IF NOT EXISTS sessions (
            session_id TEXT PRIMARY KEY,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL,
            message_count INTEGER DEFAULT 0
        );
    )";
    
    // 创建消息表
    const char* createMessagesTable = R"(
        CREATE TABLE IF NOT EXISTS messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            session_id TEXT NOT NULL,
            role TEXT NOT NULL,
            content TEXT NOT NULL,
            timestamp TEXT NOT NULL,
            FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE
        );
    )";
    
    // 创建索引
    const char* createIndex = R"(
        CREATE INDEX IF NOT EXISTS idx_messages_session 
        ON messages(session_id, timestamp);
    )";
    
    char* errMsg = nullptr;
    
    // 执行创建会话表
    int rc = sqlite3_exec(m_db, createSessionsTable, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to create sessions table: " + std::string(errMsg));
        sqlite3_free(errMsg);
        return false;
    }
    
    // 执行创建消息表
    rc = sqlite3_exec(m_db, createMessagesTable, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to create messages table: " + std::string(errMsg));
        sqlite3_free(errMsg);
        return false;
    }
    
    // 创建索引
    rc = sqlite3_exec(m_db, createIndex, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to create index: " + std::string(errMsg));
        sqlite3_free(errMsg);
        return false;
    }
    
    LOG_INFO("Database tables created successfully");
    return true;
}

std::string Database::getCurrentTimestamp() {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

bool Database::createSession(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        LOG_ERROR("Database not initialized");
        return false;
    }
    
    std::string timestamp = getCurrentTimestamp();
    const char* sql = "INSERT INTO sessions (session_id, created_at, updated_at, message_count) VALUES (?, ?, ?, 0);";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: " + std::string(sqlite3_errmsg(m_db)));
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, timestamp.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, timestamp.c_str(), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to create session: " + std::string(sqlite3_errmsg(m_db)));
        return false;
    }
    
    LOG_INFO("Session created: " + sessionId);
    return true;
}

bool Database::sessionExists(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        return false;
    }
    
    const char* sql = "SELECT COUNT(*) FROM sessions WHERE session_id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, SQLITE_TRANSIENT);
    
    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        exists = sqlite3_column_int(stmt, 0) > 0;
    }
    
    sqlite3_finalize(stmt);
    return exists;
}

std::vector<Session> Database::getAllSessions() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<Session> sessions;
    
    if (!m_initialized) {
        return sessions;
    }
    
    const char* sql = "SELECT session_id, created_at, updated_at, message_count FROM sessions ORDER BY updated_at DESC;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: " + std::string(sqlite3_errmsg(m_db)));
        return sessions;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Session session;
        session.sessionId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        session.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        session.updatedAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        session.messageCount = sqlite3_column_int(stmt, 3);
        sessions.push_back(session);
    }
    
    sqlite3_finalize(stmt);
    return sessions;
}

Session Database::getSession(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    Session session;
    
    if (!m_initialized) {
        return session;
    }
    
    const char* sql = "SELECT session_id, created_at, updated_at, message_count FROM sessions WHERE session_id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return session;
    }
    
    sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        session.sessionId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        session.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        session.updatedAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        session.messageCount = sqlite3_column_int(stmt, 3);
    }
    
    sqlite3_finalize(stmt);
    return session;
}

bool Database::deleteSession(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        return false;
    }
    
    // 先删除消息
    const char* deleteMessages = "DELETE FROM messages WHERE session_id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(m_db, deleteMessages, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    
    // 删除会话
    const char* deleteSession = "DELETE FROM sessions WHERE session_id = ?;";
    rc = sqlite3_prepare_v2(m_db, deleteSession, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc == SQLITE_DONE) {
        LOG_INFO("Session deleted: " + sessionId);
        return true;
    }
    return false;
}

bool Database::addMessage(const std::string& sessionId, const std::string& role, const std::string& content) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        LOG_ERROR("Database not initialized");
        return false;
    }
    
    std::string timestamp = getCurrentTimestamp();
    
    // 插入消息
    const char* insertMsg = "INSERT INTO messages (session_id, role, content, timestamp) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(m_db, insertMsg, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: " + std::string(sqlite3_errmsg(m_db)));
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, role.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, content.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, timestamp.c_str(), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to add message: " + std::string(sqlite3_errmsg(m_db)));
        return false;
    }
    
    // 更新会话的更新时间和消息数
    const char* updateSession = "UPDATE sessions SET updated_at = ?, message_count = message_count + 1 WHERE session_id = ?;";
    rc = sqlite3_prepare_v2(m_db, updateSession, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, timestamp.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, sessionId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    
    return true;
}

std::vector<Message> Database::getMessages(const std::string& sessionId, int limit) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<Message> messages;
    
    if (!m_initialized) {
        return messages;
    }
    
    const char* sql = "SELECT id, session_id, role, content, timestamp FROM messages WHERE session_id = ? ORDER BY timestamp ASC LIMIT ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: " + std::string(sqlite3_errmsg(m_db)));
        return messages;
    }
    
    sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, limit);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Message msg;
        msg.id = sqlite3_column_int(stmt, 0);
        msg.sessionId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        msg.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        msg.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        msg.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        messages.push_back(msg);
    }
    
    sqlite3_finalize(stmt);
    return messages;
}

std::vector<Message> Database::getRecentMessages(const std::string& sessionId, int count) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<Message> messages;
    
    if (!m_initialized) {
        return messages;
    }
    
    const char* sql = "SELECT id, session_id, role, content, timestamp FROM messages WHERE session_id = ? ORDER BY timestamp DESC LIMIT ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return messages;
    }
    
    sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, count);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Message msg;
        msg.id = sqlite3_column_int(stmt, 0);
        msg.sessionId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        msg.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        msg.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        msg.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        messages.insert(messages.begin(), msg);  // 反转顺序
    }
    
    sqlite3_finalize(stmt);
    return messages;
}

int Database::getMessageCount(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        return 0;
    }
    
    const char* sql = "SELECT COUNT(*) FROM messages WHERE session_id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, SQLITE_TRANSIENT);
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count;
}

int Database::getTotalSessions() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        return 0;
    }
    
    const char* sql = "SELECT COUNT(*) FROM sessions;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return 0;
    }
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count;
}

int Database::getTotalMessages() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        return 0;
    }
    
    const char* sql = "SELECT COUNT(*) FROM messages;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return 0;
    }
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count;
}
