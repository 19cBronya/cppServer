#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <cstdint>
#include <memory>
#include "router.h"

class Server {
public:
    Server(uint16_t port);
    ~Server();
    
    bool start();
    void stop();
    void run();
    
    bool isRunning() const { return m_running; }
    
    // 获取路由器，用于注册路由
    Router& getRouter() { return m_router; }

private:
    uint16_t m_port;
    int m_listenFd;
    bool m_running;
    Router m_router;
    
    bool createSocket();
    bool bindAndListen();
    void cleanup();
    
    // 处理客户端连接
    void handleClient(int clientFd);
    
    // 读取 HTTP 请求
    std::string readRequest(int clientFd);
};

#endif // SERVER_H

