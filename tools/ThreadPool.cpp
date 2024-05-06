
#include "ThreadPool.h"


ThreadPool::ThreadWorker::ThreadWorker(ThreadPool *pool, const int id) :  m_id(id), m_pool(pool)
{
}

void ThreadPool::ThreadWorker::operator()()
{
	bool dequeued; // 是否正在取出队列中元素

	while (!m_pool->m_shutdown)
	{
		{
			Task func; // 定义基础函数类func
		    {
		        // 为线程环境加锁，互访问工作线程的休眠和唤醒
		        std::unique_lock<std::mutex> lock(m_pool->m_conditional_mutex);

		        // 如果任务队列为空，阻塞当前线程
		        if (m_pool->m_queue.empty())
		        {
		        	//break;
		            m_pool->m_conditional_lock.wait(lock); // 等待条件变量通知，开启线程
		        }

				m_pool->m_idleThreadSize--;
		        // 取出任务队列中的元素
		        dequeued = m_pool->m_queue.dequeue(func);
		    }

		    // 如果成功取出，执行工作函数....
		    if(dequeued)
		    {
		    	func();
		    }
			m_pool->m_idleThreadSize++;
		}
	}
}


ThreadPool& ThreadPool::Instance()
{
	static ThreadPool g_ThreadPool;
	return g_ThreadPool;
}

ThreadPool::ThreadPool(const int n_threads):m_shutdown(false)
{
	int num = std::thread::hardware_concurrency();
	m_initThreadSize = n_threads > num ? num : n_threads;
	m_threadSizeThreshHold = num + 1;
	int priorityNum = m_initThreadSize < 4 ? 1: (m_initThreadSize/4);
	for (int i = 0; i < m_initThreadSize; ++i)
    {
		std::thread t(ThreadWorker(this, i));
		// if(priorityNum > i)
		// {
		// 	// 修改线程的调度策略和优先级
		// 	int policy = SCHED_FIFO;//SCHED_RR; // 设置为轮流调度
		// 	int priority = 10; // 设置优先级为 10
		// 	pthread_attr_t attr;
		// 	pthread_attr_init(&attr);
		// 	pthread_attr_setschedpolicy(&attr, policy);
		// 	sched_param param;
		// 	param.sched_priority = priority;
		// 	pthread_attr_setschedparam(&attr, &param);
		// 	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
		// 	pthread_t thread_handle = t.native_handle();
		// 	pthread_setschedparam(thread_handle, policy, &param);
		// }
#ifndef _WIN32		
		cpu_set_t cpuset;
		CPU_ZERO(&cpuset);
		CPU_SET(i,&cpuset);
		int rc =pthread_setaffinity_np(t.native_handle(),sizeof(cpu_set_t), &cpuset);
		if (rc != 0)
		{
			LogOut("THREAD", L_ERROR, "Error calling pthread_setaffinity_np:%d", rc);
		}
#endif
        m_threads.push_back(std::move(t)); // 分配工作线程
		m_curThreadSize++;
		m_idleThreadSize++;
	}
}
ThreadPool::~ThreadPool()
{
	shutdown();
}
void ThreadPool::shutdown()
{
    m_shutdown = true;
    m_conditional_lock.notify_all(); // 通知，唤醒所有工作线程

    for(size_t i = 0; i < m_threads.size(); ++i)
    {
        if (m_threads.at(i).joinable()) // 判断线程是否在等待
        {
            m_threads.at(i).join(); // 将线程加入到等待队列
        }
    }
}


#if 0

std::random_device rd; // 真实随机数产生器

std::mt19937 mt(rd()); //生成计算随机数mt

std::uniform_int_distribution<int> dist(-1000, 1000); //生成-1000到1000之间的离散均匀分布数

auto rnd = std::bind(dist, mt);

// 设置线程睡眠时间
void simulate_hard_computation()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(2000 + rnd()));
}

// 添加两个数字的简单函数并打印结果
void multiply(const int a, const int b)
{
	simulate_hard_computation();
	const int res = a * b;
	std::cout << a << " * " << b << " = " << res << std::endl;
}

// 添加并输出结果
void multiply_output(int &out, const int a, const int b)
{
	simulate_hard_computation();
	out = a * b;
	std::cout << a << " * " << b << " = " << out << std::endl;
}

// 结果返回
int multiply_return(const int a, const int b)
{
	simulate_hard_computation();
	const int res = a * b;
	std::cout << a << " * " << b << " = " << res << std::endl;
	return res;
}
void helloword()
{
	std::cout << "hello word!" << std::endl;
}
class test
{
public:
	test(){};
	~test(){}
	void printf()
	{
		std::cout << "hello word!" << std::endl;
	}
};
void example()
{
	// 创建3个线程的线程池
	ThreadPool pool(3);

	// 初始化线程池
	pool.init();

	// 提交乘法操作，总共30个
	for (int i = 1; i <= 3; ++i)
		for (int j = 1; j <= 10; ++j)
		{
			pool.submit(multiply, i, j);
		}

	// 使用ref传递的输出参数提交函数
	int output_ref;
	auto future1 = pool.submit(multiply_output, std::ref(output_ref), 5, 6);

	// 等待乘法输出完成
	future1.get();
	std::cout << "Last operation result is equals to " << output_ref << std::endl;

	// 使用return参数提交函数
	auto future2 = pool.submit(multiply_return, 5, 3);

	// 等待乘法输出完成
	int res = future2.get();
	std::cout << "Last operation result is equals to " << res << std::endl;

	pool.submit(helloword);

	//std::
	test a;
	pool.submit(std::bind(&test::printf, &a));

	// 关闭线程池
	pool.shutdown();
}
int main()
{
	example();

	return 0;
}
#endif

