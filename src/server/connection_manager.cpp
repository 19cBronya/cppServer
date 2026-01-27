#include "server/connection_manager.h"
#include "logger/logger.h"

ConnectionManager::ConnectionManager(size_t maxConnections, 
                                   int timeoutSeconds,
                                   size_t maxRequestsPerConnection)
    : m_maxConnections(maxConnections)
    , m_timeoutSeconds(timeoutSeconds)
    , m_maxRequestsPerConnection(maxRequestsPerConnection)
    , m_rejectedCount(0) {
    
    LOG_INFO("ConnectionManager created (max_connections=" + 
             std::to_string(maxConnections) + ", timeout=" + 
             std::to_string(timeoutSeconds) + "s)");
}

ConnectionManager::~ConnectionManager() {
    LOG_INFO("ConnectionManager destroyed (rejected_count=" + 
             std::to_string(m_rejectedCount.load()) + ")");
}

bool ConnectionManager::addConnection(int fd) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // 检查是否超过最大连接数
    if (m_connections.size() >= m_maxConnections) {
        LOG_WARN("Connection limit reached (" + std::to_string(m_maxConnections) + 
                "), rejecting fd " + std::to_string(fd));
        m_rejectedCount++;
        return false;
    }
    
    ConnectionInfo info;
    info.fd = fd;
    m_connections[fd] = info;
    
    LOG_DEBUG("Connection added: fd=" + std::to_string(fd) + 
             ", total=" + std::to_string(m_connections.size()));
    
    return true;
}

void ConnectionManager::removeConnection(int fd) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_connections.find(fd);
    if (it != m_connections.end()) {
        m_connections.erase(it);
        LOG_DEBUG("Connection removed: fd=" + std::to_string(fd) + 
                 ", total=" + std::to_string(m_connections.size()));
    }
}

void ConnectionManager::updateActivity(int fd) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_connections.find(fd);
    if (it != m_connections.end()) {
        it->second.lastActiveTime = std::chrono::steady_clock::now();
    }
}

void ConnectionManager::incrementRequestCount(int fd) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_connections.find(fd);
    if (it != m_connections.end()) {
        it->second.requestCount++;
    }
}

bool ConnectionManager::isTimeout(int fd) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_connections.find(fd);
    if (it == m_connections.end()) {
        return false;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        now - it->second.lastActiveTime
    ).count();
    
    return elapsed >= m_timeoutSeconds;
}

bool ConnectionManager::isOverLimit(int fd) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_connections.find(fd);
    if (it == m_connections.end()) {
        return false;
    }
    
    return it->second.requestCount >= m_maxRequestsPerConnection;
}

std::vector<int> ConnectionManager::getTimeoutConnections() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<int> timeoutFds;
    auto now = std::chrono::steady_clock::now();
    
    for (const auto& pair : m_connections) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - pair.second.lastActiveTime
        ).count();
        
        if (elapsed >= m_timeoutSeconds) {
            timeoutFds.push_back(pair.first);
        }
    }
    
    return timeoutFds;
}

bool ConnectionManager::canAcceptConnection() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_connections.size() < m_maxConnections;
}
