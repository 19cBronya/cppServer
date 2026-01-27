#include "utils/thread_pool.h"
#include "logger/logger.h"

ThreadPool::ThreadPool(size_t numThreads) : m_stop(false) {
    // 创建工作线程
    for (size_t i = 0; i < numThreads; ++i) {
        m_threads.emplace_back([this, i] {
            LOG_INFO("Worker thread " + std::to_string(i) + " started");
            this->workerThread();
            LOG_INFO("Worker thread " + std::to_string(i) + " stopped");
        });
    }
    
    LOG_INFO("ThreadPool created with " + std::to_string(numThreads) + " threads");
}

ThreadPool::~ThreadPool() {
    shutdown();
}

void ThreadPool::shutdown() {
    if (m_stop) {
        return;
    }
    
    LOG_INFO("Shutting down ThreadPool...");
    
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_stop = true;
    }
    
    // 唤醒所有等待的线程
    m_condition.notify_all();
    
    // 等待所有线程完成
    for (std::thread& thread : m_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    LOG_INFO("ThreadPool shutdown complete. Remaining tasks: " + 
             std::to_string(m_tasks.size()));
}

void ThreadPool::workerThread() {
    while (true) {
        std::function<void()> task;
        
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            
            // 等待任务或停止信号
            m_condition.wait(lock, [this] {
                return m_stop || !m_tasks.empty();
            });
            
            // 如果停止且没有任务，退出
            if (m_stop && m_tasks.empty()) {
                return;
            }
            
            // 如果有任务，取出任务
            if (!m_tasks.empty()) {
                task = std::move(m_tasks.front());
                m_tasks.pop();
            }
        }
        
        // 执行任务（在锁外执行）
        if (task) {
            try {
                task();
            } catch (const std::exception& e) {
                LOG_ERROR("Task execution failed: " + std::string(e.what()));
            } catch (...) {
                LOG_ERROR("Task execution failed with unknown exception");
            }
        }
    }
}
