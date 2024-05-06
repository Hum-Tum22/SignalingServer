#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <list>
#include <future>
#include <condition_variable>
#include <functional>
#include <unordered_map>
#include <cmath>
#include <random>
#include "SafeQueue.h"
#include "SelfLog.h"
#include <string.h>

using Task = std::function<void()>;


class ThreadPool //: public Singleton<ThreadPool>
{
private:
    class ThreadWorker // 内置线程工作类
    {
    public:
        // 构造函数
        ThreadWorker(ThreadPool *pool, const int id);

        // 重载()操作
        void operator()();
    private:
        int m_id; // 工作id

        ThreadPool *m_pool; // 所属线程池
    };

    bool m_shutdown; // 线程池是否关闭

    SafeQueue<Task> m_queue; // 执行函数安全队列，即任务队列

    std::vector<std::thread> m_threads; // 工作线程队列
    
    //初始的线程数量
	int m_initThreadSize;
	
    //记录当前线程池里面线程的总数量
	std::atomic_int m_curThreadSize;

	//线程数量上限阈值
	int m_threadSizeThreshHold;

	//记录空闲线程的数量
	std::atomic_int m_idleThreadSize;

    std::mutex m_conditional_mutex; // 线程休眠锁互斥变量

    std::condition_variable m_conditional_lock; // 线程环境锁，可以让线程处于休眠或者唤醒状态

public:
    // 线程池构造函数
    ThreadPool(const int n_threads = 8);

    ThreadPool(const ThreadPool &) = delete;

    ThreadPool(ThreadPool &&) = delete;
	~ThreadPool();

    ThreadPool &operator=(const ThreadPool &) = delete;

    ThreadPool &operator=(ThreadPool &&) = delete;

	static ThreadPool& Instance();
    // Waits until threads finish their current task and shutdowns the pool
    void shutdown();

    // Submit a function to be executed asynchronously by the pool
    template <typename F, typename... Args>
    auto submit(F &&f, Args &&...args) -> std::future<decltype(f(args...))>
    {
        // Create a function with bounded parameter ready to execute
        LogOut("THREAD", L_DEBUG, "threadPool submit queue size:%d\n", m_queue.size());
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...); // 连接函数和参数定义，特殊函数类型，避免左右值错误
        // LogOut("MQTT", L_DEBUG, "111 threadPool submit queue size:%d\n", m_queue.size());
        // Encapsulate it into a shared pointer in order to be able to copy construct
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
        // LogOut("MQTT", L_DEBUG, "222 threadPool submit queue size:%d\n", m_queue.size());
        // Warp packaged task into void function
        std::function<void()> warpper_func = [task_ptr]()
        {
            (*task_ptr)();
        };
        // LogOut("MQTT", L_DEBUG, "333 threadPool submit queue size:%d\n", m_queue.size());
        // 队列通用安全封包函数，并压入安全队列
        m_queue.enqueue(warpper_func);
        // LogOut("MQTT", L_DEBUG, "444 threadPool submit queue size:%d\n", m_queue.size());
        // 唤醒一个等待中的线程
        m_conditional_lock.notify_one();
        LogOut("THREAD", L_DEBUG, "555 threadPool submit queue size:%d\n", m_queue.size());
		//std::cout << "idle thread:" << m_idleThreadSize << " task count:" << m_queue.size() << std::endl;
        // 返回先前注册的任务指针
        return task_ptr->get_future();
    }
};
	
#endif
