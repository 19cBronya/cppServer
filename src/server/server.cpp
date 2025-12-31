#include "server/server.h"
#include "logger/logger.h"
#include "utils/signal_handler.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <csignal>
#include <cstring>
#include <cerrno>

Server::Server(uint16_t port) : m_port(port), m_listenFd(-1), m_running(false) {
}

Server::~Server() {
    cleanup();
}

bool Server::createSocket() {
    m_listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenFd < 0) {
        LOG_ERROR("Failed to create socket: " + std::string(strerror(errno)));
        return false;
    }
    
    // 设置 SO_REUSEADDR
    int opt = 1;
    if (setsockopt(m_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        LOG_ERROR("Failed to set SO_REUSEADDR: " + std::string(strerror(errno)));
        close(m_listenFd);
        m_listenFd = -1;
        return false;
    }
    
    return true;
}

bool Server::bindAndListen() {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(m_port);
    
    if (bind(m_listenFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        LOG_ERROR("Failed to bind to port " + std::to_string(m_port) + ": " + std::string(strerror(errno)));
        return false;
    }
    
    if (listen(m_listenFd, 128) < 0) {
        LOG_ERROR("Failed to listen: " + std::string(strerror(errno)));
        return false;
    }
    
    return true;
}

bool Server::start() {
    if (m_running) {
        LOG_WARN("Server is already running");
        return false;
    }
    
    if (!createSocket()) {
        return false;
    }
    
    if (!bindAndListen()) {
        cleanup();
        return false;
    }
    
    m_running = true;
    LOG_INFO("Server started on port " + std::to_string(m_port));
    return true;
}

void Server::stop() {
    if (!m_running) {
        return;
    }
    
    m_running = false;
    LOG_INFO("Server stopping...");
    cleanup();
}

void Server::run() {
    if (!m_running) {
        LOG_ERROR("Server is not running. Call start() first.");
        return;
    }
    
    LOG_INFO("Server is running. Press Ctrl+C to stop.");
    
    // 注册信号处理
    SignalHandler& signalHandler = SignalHandler::getInstance();
    signalHandler.registerHandler(SIGINT, [this]() {
        LOG_INFO("Received SIGINT, shutting down gracefully...");
        this->stop();
    });
    signalHandler.setup();
    
    // 简单的 accept 循环（后续会用 epoll 替换）
    while (m_running && !signalHandler.shouldExit()) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        
        int clientFd = accept(m_listenFd, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientFd < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                continue;
            }
            if (m_running) {
                LOG_ERROR("Failed to accept connection: " + std::string(strerror(errno)));
            }
            break;
        }
        
        char clientIp[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
        LOG_INFO("Accepted connection from " + std::string(clientIp) + ":" + std::to_string(ntohs(clientAddr.sin_port)));
        
        // 简单的响应（后续会实现 HTTP 解析）
        const char* response = "HTTP/1.1 200 OK\r\n"
                              "Content-Type: text/plain\r\n"
                              "Content-Length: 13\r\n"
                              "\r\n"
                              "Hello, World!";
        send(clientFd, response, strlen(response), 0);
        close(clientFd);
    }
    
    LOG_INFO("Server stopped");
}

void Server::cleanup() {
    if (m_listenFd >= 0) {
        close(m_listenFd);
        m_listenFd = -1;
    }
}

