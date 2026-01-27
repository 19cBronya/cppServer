#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <future>

// 线程池类：管理工作线程和任务队列
class ThreadPool {
public:
    // 构造函数：创建指定数量的工作线程
    explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency());
    
    // 析构函数：等待所有任务完成并关闭线程池
    ~ThreadPool();
    
    // 提交任务到线程池
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;
    
    // 获取线程池大小
    size_t size() const { return m_threads.size(); }
    
    // 获取当前队列中的任务数
    size_t queueSize() const { 
        std::lock_guard<std::mutex> lock(m_queueMutex);
        return m_tasks.size(); 
    }
    
    // 检查线程池是否正在运行
    bool isRunning() const { return !m_stop; }
    
    // 停止线程池（不接受新任务，等待现有任务完成）
    void shutdown();

private:
    // 工作线程
    std::vector<std::thread> m_threads;
    
    // 任务队列
    std::queue<std::function<void()>> m_tasks;
    
    // 同步原语
    mutable std::mutex m_queueMutex;
    std::condition_variable m_condition;
    
    // 停止标志
    std::atomic<bool> m_stop;
    
    // 工作线程函数
    void workerThread();
};

// 模板方法实现必须在头文件中
template<typename F, typename... Args>
auto ThreadPool::submit(F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type> {
    
    using return_type = typename std::result_of<F(Args...)>::type;
    
    // 创建一个 packaged_task
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    
    std::future<return_type> result = task->get_future();
    
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        
        // 不允许在停止后提交任务
        if (m_stop) {
            throw std::runtime_error("Cannot submit task to stopped ThreadPool");
        }
        
        // 将任务添加到队列
        m_tasks.emplace([task]() { (*task)(); });
    }
    
    // 通知一个等待的线程
    m_condition.notify_one();
    
    return result;
}

#endif // THREAD_POOL_H
