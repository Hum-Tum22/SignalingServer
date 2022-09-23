#ifndef I_THREADPOOL_H_
#define I_THREADPOOL_H_

#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <unordered_map>
#include <iostream>
#include <future>
#include "any.h"
#include "CTask.h"

namespace ownThreadPool
{

//定义一个信号量 Semaphore在C++20中已经提供
class Semaphore
{
public:
	//构造函数
	Semaphore(int limit = 0)
		:m_resLimit(limit)
		, m_isExit(false)
	{

	}

	~Semaphore()
	{
		m_isExit = true;
	}

	//获取一个信号量资源
	void wait()
	{
		if (m_isExit)
			return;
		std::unique_lock<std::mutex> lock(m_mtx);
		//等待信号量有资源，没有资源的话 阻塞当前线程
		m_cond.wait(lock, [&]()->bool {return m_resLimit > 0; });
		m_resLimit--;
	}

	//增加一个信号量资源
	void post()
	{
		if (m_isExit)
			return;
		std::unique_lock<std::mutex> lock(m_mtx);
		m_resLimit++;
		m_cond.notify_all();
	}

private:
	std::atomic_bool m_isExit;
	int m_resLimit;
	std::mutex m_mtx;
	std::condition_variable m_cond;
};


class Result;


//提交任务到线程池完成后的返回值类型
//class Result
//{
//public:
//	Result() {};
//	Result(const Result& res);
//	Result(std::shared_ptr<Task> task, bool isValid = true);
//
//	Result& operator=(Result&& rhs) noexcept;
//
//	//setVal() 获取任务执行完的返回值
//	void SetVal(ownAny::Any any);
//
//	//get() 用户调用这个方法获取Task的返回值
//	ownAny::Any get();
//
//	~Result()// = default;
//	{
//		std::cout << "del Result:" << this << std::endl;
//	}
//
//private:
//	std::shared_ptr<Task> m_task;	//指向对应任务返回值的任务对象
//	ownAny::Any m_any;						//存储任务的返回值
//	Semaphore m_sem;				//线程通信信号量
//	std::atomic_bool m_isValid;		//返回值是否有效
//};

//线程池支持的模式
enum class PoolMode
{
	MODE_FIXED,		//固定数量的线程
	MODE_CACHED,	//线程数量可以动态增长
};

//线程类型
class myThread
{
public:
	enum   threadstate
	{
		_t_starte,
		_t_busy,
		_t_idle,
		_t_end
	};
	//线程函数对象类型
	using ThreadFunc = std::function<void(int)>;
	// 线程构造
	myThread(ThreadFunc func);
	// 线程析构
	~myThread();
	// 启动线程
	void start();

	//获取线程ID
	int getId()const;
	void setThreadState(threadstate state) { m_ThreadState = state; }
	threadstate getThreadState() { return m_ThreadState; }
private:
	ThreadFunc m_func;
	static int m_generateId;
	int m_threadId;		//保存线程id
	threadstate m_ThreadState;
};

//线程池类型
class myThreadPool
{
public:
	//线程池构造
	myThreadPool();
	//线程池析构
	~myThreadPool();

	//设置线程池的工作模式
	void setMode(PoolMode mode);

	//设置task任务队列上线的阈值
	void setTaskQueMaxThrshHold(int threshhold);
	//给线程池提交任务
	bool submitTask(std::shared_ptr<ownTask::CTask> sp);
	//给线程池提交任务
	//使用可变参模板变参 让submitTask可以接受任意任务函数和任意数量的参数
	//Result submitTask(std::shared_ptr<Task> sp);
	template<typename Func, typename... Args>
	auto submit(Func&& func, Args&&... args)->std::future<decltype(func(args...))>
	{
		//打包任务 放入任务队列
		using RType = decltype(func(args...));
		auto task = std::make_shared<std::packaged_task<RType()>>(std::bind(std::forward<Func>(func),
			std::forward<Args>(args)...));

		std::future<RType> result = task->get_future();

		//获取锁
		std::unique_lock<std::mutex> lock(m_taskQueMtx);
		//用户提交任务，最长不可超过一秒，否则算提交任务失败
		if (!m_notFull.wait_for(lock, std::chrono::seconds(1),
			[&]()->bool {return m_taskque.size() < (size_t)m_taskqueMaxThresHold; }))
		{
			//表示等待一秒 条件依然没有满足
			std::cerr << "task queue is full, submit task failed." << std::endl;
			auto task = std::make_shared<std::packaged_task<RType()>>(
				[]()->RType {return RType(); });
			(*task)();
			return task->get_future();
		}

		//如果有空余， 把任务放入任务队列中
		m_taskque.emplace([task]() {(*task)(); });
		m_taskSize++;

		//因为新放了任务 所以任务队列肯定不为空了，在m_notEmpty上面进行通知，赶快分配线程执行执行资源
		m_notEmpty.notify_all();

		// cached模式 任务处理比较紧急 场景：小而快的任务 
		//需要根据任务数量和空闲线程的数量，判断是否需要创建新的线程出来
		if (m_poolMode == PoolMode::MODE_CACHED && m_taskSize > m_idleThreadSize
			&& m_curThreadSize < m_threadSizeThreshHold)
		{
			std::cout << ">>> create new thread..." << std::endl;
			//创建新的线程对象
			auto ptr = std::unique_ptr<myThread>(std::bind(&myThreadPool::threadFunc, this, std::placeholders::_1));
			int threadId = ptr->getId();
			m_threads.emplace(threadId, std::move(ptr));
			//启动线程
			m_threads[threadId]->start();
			//修改线程个数相关的变量
			m_curThreadSize++;
			m_idleThreadSize++;
		}
		return result;
	}

	//设置线程池cached模式下线程阈值
	void setThreadSizeThreshHold(int threshHold);

	//开启线程池
	void start(int initThreadSize = std::thread::hardware_concurrency());

	myThreadPool(const myThreadPool&) = delete;
	myThreadPool& operator=(const myThreadPool&) = delete;

	//检查pool的运行状态
	bool checkRunningState() const;
private:
	//定义线程函数
	void threadFunc(int threadId);

private:
	//std::vector<std::unique_ptr<Thread>> m_threads;	//线程列表
	std::unordered_map <int, std::unique_ptr<myThread>> m_threads;
	//初始的线程数量
	int m_initThreadSize;

	//记录当前线程池里面线程的总数量
	std::atomic_int m_curThreadSize;

	//线程数量上限阈值
	int m_threadSizeThreshHold;

	//记录空闲线程的数量
	std::atomic_int m_idleThreadSize;

	//任务队列
	std::queue<std::shared_ptr<ownTask::CTask>> m_taskque;

	//任务的数量
	std::atomic_int m_taskSize;
	//任务队列数量上限的阈值
	int m_taskqueMaxThresHold;

	//包装任务队列的线程安全
	std::mutex m_taskQueMtx;

	//表示任务队列不满
	std::condition_variable m_notFull;
	//表示任务队列不空
	std::condition_variable m_notEmpty;

	//表示等待线程资源全部回收
	std::condition_variable m_exitCond;

	//当前线程池的工作模式
	PoolMode m_poolMode;

	//表示当前线程池的启动状态
	std::atomic_bool m_isPoolRunning;
};

ownThreadPool::myThreadPool& GetThreadPool();
}

#endif // !THREADPOOL_H_