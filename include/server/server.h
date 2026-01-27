#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <cstdint>
#include <memory>
#include <thread>
#include "router.h"
#include "epoll_reactor.h"
#include "connection_manager.h"
#include "utils/thread_pool.h"

class Server {
public:
    Server(uint16_t port, size_t numThreads = 0);
    ~Server();
    
    bool start();
    void stop();
    void run();
    
    bool isRunning() const { return m_running; }
    
    // 获取路由器，用于注册路由
    Router& getRouter() { return m_router; }
    
    // 获取统计信息
    size_t getConnectionCount() const;
    size_t getRejectedCount() const;
    size_t getThreadPoolQueueSize() const;

private:
    uint16_t m_port;
    int m_listenFd;
    bool m_running;
    Router m_router;
    
    // epoll reactor
    std::unique_ptr<EpollReactor> m_reactor;
    
    // 线程池
    std::unique_ptr<ThreadPool> m_threadPool;
    
    // 连接管理器
    std::unique_ptr<ConnectionManager> m_connManager;
    
    // 超时检查线程
    std::unique_ptr<std::thread> m_timeoutThread;
    
    bool createSocket();
    bool bindAndListen();
    void cleanup();
    
    // 设置非阻塞
    bool setNonBlocking(int fd);
    
    // epoll 事件处理器
    void handleListenEvent(int fd, uint32_t events);
    void handleClientEvent(int fd, uint32_t events);
    
    // 处理客户端连接
    void handleClient(int clientFd);
    
    // 读取 HTTP 请求
    std::string readRequest(int clientFd);
    
    // 超时检查循环
    void timeoutCheckLoop();
    
    // 关闭客户端连接
    void closeConnection(int fd);
};

#endif // SERVER_H

