#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <unordered_map>
#include <vector>
#include <mutex>
#include <chrono>
#include <atomic>

// 连接信息
struct ConnectionInfo {
    int fd;
    std::chrono::steady_clock::time_point lastActiveTime;
    std::chrono::steady_clock::time_point createTime;
    size_t requestCount;
    
    ConnectionInfo() : fd(-1), requestCount(0) {
        auto now = std::chrono::steady_clock::now();
        lastActiveTime = now;
        createTime = now;
    }
};

// 连接管理器：过载保护
class ConnectionManager {
public:
    ConnectionManager(size_t maxConnections = 10000, 
                     int timeoutSeconds = 60,
                     size_t maxRequestsPerConnection = 1000);
    
    ~ConnectionManager();
    
    // 添加连接
    bool addConnection(int fd);
    
    // 移除连接
    void removeConnection(int fd);
    
    // 更新连接活跃时间
    void updateActivity(int fd);
    
    // 增加请求计数
    void incrementRequestCount(int fd);
    
    // 检查连接是否超时
    bool isTimeout(int fd) const;
    
    // 检查连接是否超过请求限制
    bool isOverLimit(int fd) const;
    
    // 获取超时的连接列表
    std::vector<int> getTimeoutConnections() const;
    
    // 检查是否可以接受新连接（过载保护）
    bool canAcceptConnection() const;
    
    // 获取当前连接数
    size_t getConnectionCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_connections.size();
    }
    
    // 获取最大连接数
    size_t getMaxConnections() const { return m_maxConnections; }
    
    // 获取拒绝的连接数（统计）
    size_t getRejectedCount() const { return m_rejectedCount; }

private:
    std::unordered_map<int, ConnectionInfo> m_connections;
    mutable std::mutex m_mutex;
    
    size_t m_maxConnections;                    // 最大连接数
    int m_timeoutSeconds;                       // 超时时间（秒）
    size_t m_maxRequestsPerConnection;          // 每个连接的最大请求数
    std::atomic<size_t> m_rejectedCount;        // 拒绝的连接数
};

#endif // CONNECTION_MANAGER_H
