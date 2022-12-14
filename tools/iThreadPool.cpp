#include "iThreadPool.h"

#include <thread>
#include <iostream>

const int TASK_MAX_THRESHHOLD = 100000;
const int THREAD_MAX_THRESHHOLD = 1024;
const int THREAD_MAX_IDLE_TIME = 60;	//单位：秒

/*
-------------------------------
线程方法的实现
-------------------------------
*/
using namespace ownThreadPool;
int myThread::m_generateId = 0;

myThread::myThread(ThreadFunc func)
	:m_func(func)
	, m_threadId(m_generateId++)
{

}
// 线程析构
myThread::~myThread()
{

}

void myThread::start()
{
	//创建一个线程来执行一个线程函数
	std::thread t(m_func, m_threadId);	//C++11线程对象和线程函数
	t.detach();	//设置守护线程
}

int myThread::getId() const
{
	return m_threadId;
}

/*
-------------------------------
线程池方法的实现
-------------------------------
*/
//线程构造
myThreadPool::myThreadPool()
	:m_initThreadSize(std::thread::hardware_concurrency()),
	m_taskSize(0),
	m_taskqueMaxThresHold(TASK_MAX_THRESHHOLD),
	m_threadSizeThreshHold(THREAD_MAX_THRESHHOLD),
	m_poolMode(PoolMode::MODE_FIXED),
	m_isPoolRunning(false),
	m_idleThreadSize(0),
	m_curThreadSize(0)
{

}

//线程池析构
myThreadPool::~myThreadPool()
{
	m_isPoolRunning = false;

	//等待线程池里面所有的线程返回 
	std::unique_lock<std::mutex> lock(m_taskQueMtx);
	m_notEmpty.notify_all();
	m_exitCond.wait(lock, [&]()->bool {return m_threads.size() == 0; });
}

//设置线程池的工作模式
void myThreadPool::setMode(PoolMode mode)
{
	if (checkRunningState()) return;
	m_poolMode = mode;
}

//设置task任务队列上线的阈值
void myThreadPool::setTaskQueMaxThrshHold(int threshhold)
{
	m_taskqueMaxThresHold = threshhold;
}

//给线程池提交任务
bool myThreadPool::submitTask(std::shared_ptr<ownTask::CTask> sp)
{
	//获取锁
	std::unique_lock<std::mutex> lock(m_taskQueMtx);

	//线程的通信 等待任务有空余
	//用户提交任务 
	//m_notFull.wait(lock, [&]()->bool {return m_taskque.size() < TASK_MAX_THRESHHOLD; });

	//最长租塞不能超过1s 否则线程提交失败
	if (!m_notFull.wait_for(lock, std::chrono::seconds(1),[&]()->bool {return m_taskque.size() < TASK_MAX_THRESHHOLD; }))
	{
		std::cerr << "task queue is full, submit task failed." << std::endl;
		return false;// Result(sp, false);
	}

	//如果有空余，那么久放入任务队列中
	m_taskque.emplace(sp);
	m_taskSize++;

	//因为新放了任务 那么队列也就不为空了，在not_empty上进行通知 让线程赶快执行任务
	m_notEmpty.notify_one();

	//cached 任务处理比较紧急 场景：小而快的任务
	//需要根据任务数量和空闲线程的数量，判断是否需要判断新的线程
	if (m_poolMode == PoolMode::MODE_CACHED &&
		m_taskSize > m_idleThreadSize &&
		m_curThreadSize < m_threadSizeThreshHold)
	{
		std::cout << ">>>create new thread..." << std::endl;
		//创建新线程
		auto ptr = std::unique_ptr<myThread>(new myThread(std::bind(&myThreadPool::threadFunc, this, std::placeholders::_1)));
		//m_threads.emplace_back(std::move(ptr));
		int threadId = ptr->getId();
		m_threads.emplace(threadId, std::move(ptr));
		//启动线程
		m_threads[threadId]->start();
		//修改线程个数相关的变量
		m_curThreadSize++;
		m_idleThreadSize++;
	}

	return true;// Result(sp);
}

void myThreadPool::setThreadSizeThreshHold(int threshHold)
{
	if (checkRunningState())
		return;
	if (m_poolMode == PoolMode::MODE_CACHED)
	{
		m_threadSizeThreshHold = threshHold;
	}
}

//开启线程池
void myThreadPool::start(int initThreadSize)
{
	//设置线程池的运行状态
	m_isPoolRunning = true;
	//记录初始线程个数
	m_initThreadSize = initThreadSize;
	m_curThreadSize = initThreadSize;

	//创建线程对象
	for (int i = 0; i < m_initThreadSize; i++)
	{
		//创建thread线程对象的时候
		//把线程函数给到thread线程对象
		//m_threads.emplace_back(new Thread(std::bind(&myThreadPool::threadFunc, this)));
		auto ptr = std::unique_ptr<myThread>(new myThread(std::bind(&myThreadPool::threadFunc, this, std::placeholders::_1)));
		//m_threads.emplace_back(std::move(ptr));
		int threadId = ptr->getId();
		m_threads.emplace(threadId, std::move(ptr));
	}

	//启动所有线程 std::vector<Thread*> m_threads;
	for (int i = 0; i < m_initThreadSize; i++)
	{
		m_threads[i]->start();  //需要去执行一个线程函数
		m_idleThreadSize++;		//记录初始空闲线程的数量
	}
}

//定义线程函数
void myThreadPool::threadFunc(int threadId)
{
	auto lastTime = std::chrono::high_resolution_clock().now();
	
	//m_threads[threadId]->
	//m_ThreadState = _t_starte;
	while (true)
	{
		std::shared_ptr<ownTask::CTask> task;
		{
			//获取锁
			std::unique_lock<std::mutex> lock(m_taskQueMtx);

			std::cout << "tid:" << std::this_thread::get_id() << "正在尝试获取任务..." << std::endl;

			//cached模式下 有可能已经创建了很多的线程 但是空闲时间超过60s
			//应该把多余的线程回收结束掉（超过m_initThreadSize数量的线程进行回收）
			//当前时间 - 上一次线程执行的事件 > 60s

			// 每一秒钟返回一次 区分：超时返回和有任务待执行返回
			while (m_taskque.size() == 0)
			{
				if (!m_isPoolRunning)
				{
					m_threads.erase(threadId);
					std::cout << "threadId"
						<< std::this_thread::get_id()
						<< "exit" << std::endl;
					m_exitCond.notify_all();
					return;
				}
				if (m_poolMode == PoolMode::MODE_CACHED)
				{
					if (std::cv_status::timeout == m_notEmpty.wait_for(lock, std::chrono::seconds(1)))
					{
						auto now = std::chrono::high_resolution_clock().now();
						auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
						if (dur.count() >= THREAD_MAX_IDLE_TIME && m_curThreadSize > m_initThreadSize)
						{
							//回收当前线程
							//记录线程数量的相关变量值的修改
							//把线程对象从线程列表容器中删除 没有办法匹配threadFunc对应的是哪一个线程对象
							m_threads.erase(threadId);
							m_curThreadSize--;
							m_idleThreadSize--;
							std::cout << "threadId" << std::this_thread::get_id() << "exit" << std::endl;
							return;
						}
					}
				}
				else
				{
					//等待notEmpty条件
					m_notEmpty.wait(lock);
				}
				//线程池要结束 回收线程资源	

			}
			m_idleThreadSize--;

			std::cout << "tid:" << std::this_thread::get_id() << "获取任务成功" << std::endl;

			//从任务队列中获取一个任务
			task = m_taskque.front();
			m_taskque.pop();
			m_taskSize--;

			//如果依然有其他任务 则通知其他线程执行任务
			if (m_taskque.size() > 0)
			{
				m_notEmpty.notify_one();
			}

			//取出一个任务进行通知，通知可以继续提交任务
			m_notFull.notify_all();
		}//及时释放锁
		if (task != nullptr)
		{
			task->TaskRun();
			task->TaskClose();
			task->TaskDestory();
		}
		m_idleThreadSize++;
		//更新线程执行完成任务的时间
		lastTime = std::chrono::high_resolution_clock().now();
	}
}

bool myThreadPool::checkRunningState() const
{
	return m_isPoolRunning;
}

ownThreadPool::myThreadPool g_MyThreadPool;

ownThreadPool::myThreadPool& ownThreadPool::GetThreadPool()
{
	if (!g_MyThreadPool.checkRunningState())
	{
		g_MyThreadPool.start(std::thread::hardware_concurrency());
	}
	return g_MyThreadPool;
}