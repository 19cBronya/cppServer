#include "server/server.h"
#include "logger/logger.h"
#include "utils/signal_handler.h"
#include "http_parser.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <csignal>
#include <cstring>
#include <cerrno>
#include <sstream>

Server::Server(uint16_t port, size_t numThreads) 
    : m_port(port)
    , m_listenFd(-1)
    , m_running(false) {
    
    // 创建线程池（默认使用硬件并发数）
    if (numThreads == 0) {
        numThreads = std::thread::hardware_concurrency();
        if (numThreads == 0) numThreads = 4;  // 回退到 4 个线程
    }
    m_threadPool = std::make_unique<ThreadPool>(numThreads);
    
    // 创建 epoll reactor
    m_reactor = std::make_unique<EpollReactor>();
    
    // 创建连接管理器（最大 10000 连接，60 秒超时）
    m_connManager = std::make_unique<ConnectionManager>(10000, 60, 10000);
    
    LOG_INFO("Server created with " + std::to_string(numThreads) + " worker threads");
}

Server::~Server() {
    stop();
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
    
    // 设置监听 socket 为非阻塞
    if (!setNonBlocking(m_listenFd)) {
        LOG_ERROR("Failed to set listen socket non-blocking");
        cleanup();
        return false;
    }
    
    // 将监听 socket 添加到 epoll
    if (!m_reactor->addFd(m_listenFd, EPOLLIN, 
        [this](int fd, uint32_t events) { this->handleListenEvent(fd, events); })) {
        LOG_ERROR("Failed to add listen socket to epoll");
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
    
    // 停止 reactor
    if (m_reactor) {
        m_reactor->stop();
    }
    
    // 停止超时检查线程
    if (m_timeoutThread && m_timeoutThread->joinable()) {
        m_timeoutThread->join();
    }
    
    // 停止线程池
    if (m_threadPool) {
        m_threadPool->shutdown();
    }
    
    cleanup();
}

void Server::run() {
    if (!m_running) {
        LOG_ERROR("Server is not running. Call start() first.");
        return;
    }
    
    LOG_INFO("Server is running (epoll + thread pool). Press Ctrl+C to stop.");
    
    // 注册信号处理
    SignalHandler& signalHandler = SignalHandler::getInstance();
    signalHandler.registerHandler(SIGINT, [this]() {
        LOG_INFO("Received SIGINT, shutting down gracefully...");
        this->stop();
    });
    signalHandler.setup();
    
    // 启动超时检查线程
    m_timeoutThread = std::make_unique<std::thread>([this]() {
        this->timeoutCheckLoop();
    });
    
    // 运行 epoll 事件循环
    m_reactor->run();
    
    LOG_INFO("Server stopped");
}

void Server::handleClient(int clientFd) {
    // 读取 HTTP 请求
    std::string rawRequest = readRequest(clientFd);
    
    if (rawRequest.empty()) {
        LOG_WARN("Empty request received");
        return;
    }
    
    // 解析 HTTP 请求
    HttpRequest request = HttpParser::parseRequest(rawRequest);
    
    // 使用路由处理请求
    HttpResponse response = m_router.handleRequest(request);
    
    // 发送响应
    std::string rawResponse = response.toString();
    send(clientFd, rawResponse.c_str(), rawResponse.length(), 0);
}

std::string Server::readRequest(int clientFd) {
    const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];
    std::ostringstream oss;
    
    // 读取数据（简单实现，后续可优化）
    ssize_t n = recv(clientFd, buffer, BUFFER_SIZE - 1, 0);
    if (n > 0) {
        buffer[n] = '\0';
        oss << buffer;
    } else if (n < 0) {
        LOG_ERROR("Failed to read from client: " + std::string(strerror(errno)));
    }
    
    return oss.str();
}

void Server::cleanup() {
    if (m_listenFd >= 0) {
        close(m_listenFd);
        m_listenFd = -1;
    }
}

bool Server::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        LOG_ERROR("Failed to get socket flags: " + std::string(strerror(errno)));
        return false;
    }
    
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        LOG_ERROR("Failed to set socket non-blocking: " + std::string(strerror(errno)));
        return false;
    }
    
    return true;
}

void Server::handleListenEvent(int fd, uint32_t events) {
    if (events & (EPOLLERR | EPOLLHUP)) {
        LOG_ERROR("Error on listen socket");
        return;
    }
    
    // 循环接受所有待处理的连接（边缘触发模式）
    while (m_running) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        
        int clientFd = accept(m_listenFd, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientFd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 所有连接都已接受
                break;
            } else if (errno == EINTR) {
                // 被信号中断，继续
                continue;
            } else {
                LOG_ERROR("Failed to accept connection: " + std::string(strerror(errno)));
                break;
            }
        }
        
        // 过载保护：检查是否可以接受新连接
        if (!m_connManager->canAcceptConnection()) {
            LOG_WARN("Connection limit reached, rejecting connection");
            close(clientFd);
            continue;
        }
        
        char clientIp[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
        LOG_DEBUG("Accepted connection from " + std::string(clientIp) + ":" + 
                 std::to_string(ntohs(clientAddr.sin_port)) + " (fd=" + std::to_string(clientFd) + ")");
        
        // 设置客户端 socket 为非阻塞
        if (!setNonBlocking(clientFd)) {
            close(clientFd);
            continue;
        }
        
        // 添加到连接管理器
        if (!m_connManager->addConnection(clientFd)) {
            close(clientFd);
            continue;
        }
        
        // 添加到 epoll
        if (!m_reactor->addFd(clientFd, EPOLLIN | EPOLLOUT, 
            [this](int fd, uint32_t events) { this->handleClientEvent(fd, events); })) {
            LOG_ERROR("Failed to add client socket to epoll");
            m_connManager->removeConnection(clientFd);
            close(clientFd);
            continue;
        }
    }
}

void Server::handleClientEvent(int fd, uint32_t events) {
    // 错误或关闭事件
    if (events & (EPOLLERR | EPOLLHUP)) {
        LOG_DEBUG("Connection closed or error on fd " + std::to_string(fd));
        closeConnection(fd);
        return;
    }
    
    // 可读事件
    if (events & EPOLLIN) {
        // 更新活跃时间
        m_connManager->updateActivity(fd);
        
        // 提交到线程池处理
        m_threadPool->submit([this, fd]() {
            this->handleClient(fd);
        });
    }
}

void Server::closeConnection(int fd) {
    m_reactor->removeFd(fd);
    m_connManager->removeConnection(fd);
    close(fd);
}

void Server::timeoutCheckLoop() {
    LOG_INFO("Timeout check thread started");
    
    while (m_running) {
        // 每 10 秒检查一次超时连接
        std::this_thread::sleep_for(std::chrono::seconds(10));
        
        if (!m_running) break;
        
        // 获取超时的连接
        auto timeoutFds = m_connManager->getTimeoutConnections();
        
        if (!timeoutFds.empty()) {
            LOG_INFO("Closing " + std::to_string(timeoutFds.size()) + " timeout connections");
            
            for (int fd : timeoutFds) {
                closeConnection(fd);
            }
        }
    }
    
    LOG_INFO("Timeout check thread stopped");
}

size_t Server::getConnectionCount() const {
    return m_connManager ? m_connManager->getConnectionCount() : 0;
}

size_t Server::getRejectedCount() const {
    return m_connManager ? m_connManager->getRejectedCount() : 0;
}

size_t Server::getThreadPoolQueueSize() const {
    return m_threadPool ? m_threadPool->queueSize() : 0;
}

