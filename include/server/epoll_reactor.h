#ifndef EPOLL_REACTOR_H
#define EPOLL_REACTOR_H

#include <sys/epoll.h>
#include <functional>
#include <unordered_map>
#include <memory>
#include <atomic>

// 事件类型
enum class EventType {
    READ = EPOLLIN,
    WRITE = EPOLLOUT,
    ERROR = EPOLLERR | EPOLLHUP
};

// 事件处理器
using EventHandler = std::function<void(int fd, uint32_t events)>;

// Epoll Reactor：事件驱动模型
class EpollReactor {
public:
    EpollReactor();
    ~EpollReactor();
    
    // 添加文件描述符到 epoll 监听
    bool addFd(int fd, uint32_t events, EventHandler handler);
    
    // 修改文件描述符的监听事件
    bool modifyFd(int fd, uint32_t events);
    
    // 从 epoll 中移除文件描述符
    bool removeFd(int fd);
    
    // 运行事件循环（阻塞）
    void run();
    
    // 停止事件循环
    void stop();
    
    // 检查是否正在运行
    bool isRunning() const { return m_running; }
    
    // 获取 epoll fd
    int getEpollFd() const { return m_epollFd; }

private:
    int m_epollFd;                                      // epoll 文件描述符
    std::atomic<bool> m_running;                        // 运行标志
    std::unordered_map<int, EventHandler> m_handlers;   // fd -> 事件处理器映射
    
    static constexpr int MAX_EVENTS = 1024;             // 最大事件数
    static constexpr int EPOLL_TIMEOUT = 1000;          // epoll_wait 超时时间（毫秒）
};

#endif // EPOLL_REACTOR_H
