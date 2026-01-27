#include "server/epoll_reactor.h"
#include "logger/logger.h"
#include <unistd.h>
#include <cstring>
#include <cerrno>

EpollReactor::EpollReactor() : m_epollFd(-1), m_running(false) {
    // 创建 epoll 实例
    m_epollFd = epoll_create1(0);
    if (m_epollFd < 0) {
        LOG_ERROR("Failed to create epoll: " + std::string(strerror(errno)));
        throw std::runtime_error("Failed to create epoll");
    }
    
    LOG_INFO("EpollReactor created (epoll_fd=" + std::to_string(m_epollFd) + ")");
}

EpollReactor::~EpollReactor() {
    stop();
    
    if (m_epollFd >= 0) {
        close(m_epollFd);
        LOG_INFO("EpollReactor destroyed");
    }
}

bool EpollReactor::addFd(int fd, uint32_t events, EventHandler handler) {
    if (fd < 0) {
        LOG_ERROR("Invalid fd: " + std::to_string(fd));
        return false;
    }
    
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = events | EPOLLET;  // 使用边缘触发模式
    ev.data.fd = fd;
    
    if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        LOG_ERROR("Failed to add fd " + std::to_string(fd) + " to epoll: " + 
                  std::string(strerror(errno)));
        return false;
    }
    
    // 保存事件处理器
    m_handlers[fd] = handler;
    
    LOG_DEBUG("Added fd " + std::to_string(fd) + " to epoll");
    return true;
}

bool EpollReactor::modifyFd(int fd, uint32_t events) {
    if (fd < 0) {
        LOG_ERROR("Invalid fd: " + std::to_string(fd));
        return false;
    }
    
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = events | EPOLLET;
    ev.data.fd = fd;
    
    if (epoll_ctl(m_epollFd, EPOLL_CTL_MOD, fd, &ev) < 0) {
        LOG_ERROR("Failed to modify fd " + std::to_string(fd) + " in epoll: " + 
                  std::string(strerror(errno)));
        return false;
    }
    
    LOG_DEBUG("Modified fd " + std::to_string(fd) + " in epoll");
    return true;
}

bool EpollReactor::removeFd(int fd) {
    if (fd < 0) {
        return false;
    }
    
    if (epoll_ctl(m_epollFd, EPOLL_CTL_DEL, fd, nullptr) < 0) {
        LOG_ERROR("Failed to remove fd " + std::to_string(fd) + " from epoll: " + 
                  std::string(strerror(errno)));
        return false;
    }
    
    // 移除事件处理器
    m_handlers.erase(fd);
    
    LOG_DEBUG("Removed fd " + std::to_string(fd) + " from epoll");
    return true;
}

void EpollReactor::run() {
    if (m_running) {
        LOG_WARN("EpollReactor is already running");
        return;
    }
    
    m_running = true;
    LOG_INFO("EpollReactor started");
    
    struct epoll_event events[MAX_EVENTS];
    
    while (m_running) {
        // 等待事件
        int nfds = epoll_wait(m_epollFd, events, MAX_EVENTS, EPOLL_TIMEOUT);
        
        if (nfds < 0) {
            if (errno == EINTR) {
                // 被信号中断，继续
                continue;
            }
            LOG_ERROR("epoll_wait failed: " + std::string(strerror(errno)));
            break;
        }
        
        // 处理事件
        for (int i = 0; i < nfds; ++i) {
            int fd = events[i].data.fd;
            uint32_t revents = events[i].events;
            
            // 查找事件处理器
            auto it = m_handlers.find(fd);
            if (it != m_handlers.end()) {
                try {
                    it->second(fd, revents);
                } catch (const std::exception& e) {
                    LOG_ERROR("Event handler failed for fd " + std::to_string(fd) + 
                             ": " + std::string(e.what()));
                } catch (...) {
                    LOG_ERROR("Event handler failed for fd " + std::to_string(fd) + 
                             " with unknown exception");
                }
            } else {
                LOG_WARN("No handler found for fd " + std::to_string(fd));
            }
        }
    }
    
    LOG_INFO("EpollReactor stopped");
}

void EpollReactor::stop() {
    if (m_running) {
        LOG_INFO("Stopping EpollReactor...");
        m_running = false;
    }
}
