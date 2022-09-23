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

//����һ���ź��� Semaphore��C++20���Ѿ��ṩ
class Semaphore
{
public:
	//���캯��
	Semaphore(int limit = 0)
		:m_resLimit(limit)
		, m_isExit(false)
	{

	}

	~Semaphore()
	{
		m_isExit = true;
	}

	//��ȡһ���ź�����Դ
	void wait()
	{
		if (m_isExit)
			return;
		std::unique_lock<std::mutex> lock(m_mtx);
		//�ȴ��ź�������Դ��û����Դ�Ļ� ������ǰ�߳�
		m_cond.wait(lock, [&]()->bool {return m_resLimit > 0; });
		m_resLimit--;
	}

	//����һ���ź�����Դ
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


//�ύ�����̳߳���ɺ�ķ���ֵ����
//class Result
//{
//public:
//	Result() {};
//	Result(const Result& res);
//	Result(std::shared_ptr<Task> task, bool isValid = true);
//
//	Result& operator=(Result&& rhs) noexcept;
//
//	//setVal() ��ȡ����ִ����ķ���ֵ
//	void SetVal(ownAny::Any any);
//
//	//get() �û��������������ȡTask�ķ���ֵ
//	ownAny::Any get();
//
//	~Result()// = default;
//	{
//		std::cout << "del Result:" << this << std::endl;
//	}
//
//private:
//	std::shared_ptr<Task> m_task;	//ָ���Ӧ���񷵻�ֵ���������
//	ownAny::Any m_any;						//�洢����ķ���ֵ
//	Semaphore m_sem;				//�߳�ͨ���ź���
//	std::atomic_bool m_isValid;		//����ֵ�Ƿ���Ч
//};

//�̳߳�֧�ֵ�ģʽ
enum class PoolMode
{
	MODE_FIXED,		//�̶��������߳�
	MODE_CACHED,	//�߳��������Զ�̬����
};

//�߳�����
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
	//�̺߳�����������
	using ThreadFunc = std::function<void(int)>;
	// �̹߳���
	myThread(ThreadFunc func);
	// �߳�����
	~myThread();
	// �����߳�
	void start();

	//��ȡ�߳�ID
	int getId()const;
	void setThreadState(threadstate state) { m_ThreadState = state; }
	threadstate getThreadState() { return m_ThreadState; }
private:
	ThreadFunc m_func;
	static int m_generateId;
	int m_threadId;		//�����߳�id
	threadstate m_ThreadState;
};

//�̳߳�����
class myThreadPool
{
public:
	//�̳߳ع���
	myThreadPool();
	//�̳߳�����
	~myThreadPool();

	//�����̳߳صĹ���ģʽ
	void setMode(PoolMode mode);

	//����task����������ߵ���ֵ
	void setTaskQueMaxThrshHold(int threshhold);
	//���̳߳��ύ����
	bool submitTask(std::shared_ptr<ownTask::CTask> sp);
	//���̳߳��ύ����
	//ʹ�ÿɱ��ģ���� ��submitTask���Խ������������������������Ĳ���
	//Result submitTask(std::shared_ptr<Task> sp);
	template<typename Func, typename... Args>
	auto submit(Func&& func, Args&&... args)->std::future<decltype(func(args...))>
	{
		//������� �����������
		using RType = decltype(func(args...));
		auto task = std::make_shared<std::packaged_task<RType()>>(std::bind(std::forward<Func>(func),
			std::forward<Args>(args)...));

		std::future<RType> result = task->get_future();

		//��ȡ��
		std::unique_lock<std::mutex> lock(m_taskQueMtx);
		//�û��ύ��������ɳ���һ�룬�������ύ����ʧ��
		if (!m_notFull.wait_for(lock, std::chrono::seconds(1),
			[&]()->bool {return m_taskque.size() < (size_t)m_taskqueMaxThresHold; }))
		{
			//��ʾ�ȴ�һ�� ������Ȼû������
			std::cerr << "task queue is full, submit task failed." << std::endl;
			auto task = std::make_shared<std::packaged_task<RType()>>(
				[]()->RType {return RType(); });
			(*task)();
			return task->get_future();
		}

		//����п��࣬ ������������������
		m_taskque.emplace([task]() {(*task)(); });
		m_taskSize++;

		//��Ϊ�·������� ����������п϶���Ϊ���ˣ���m_notEmpty�������֪ͨ���Ͽ�����߳�ִ��ִ����Դ
		m_notEmpty.notify_all();

		// cachedģʽ ������ȽϽ��� ������С��������� 
		//��Ҫ�������������Ϳ����̵߳��������ж��Ƿ���Ҫ�����µ��̳߳���
		if (m_poolMode == PoolMode::MODE_CACHED && m_taskSize > m_idleThreadSize
			&& m_curThreadSize < m_threadSizeThreshHold)
		{
			std::cout << ">>> create new thread..." << std::endl;
			//�����µ��̶߳���
			auto ptr = std::unique_ptr<myThread>(std::bind(&myThreadPool::threadFunc, this, std::placeholders::_1));
			int threadId = ptr->getId();
			m_threads.emplace(threadId, std::move(ptr));
			//�����߳�
			m_threads[threadId]->start();
			//�޸��̸߳�����صı���
			m_curThreadSize++;
			m_idleThreadSize++;
		}
		return result;
	}

	//�����̳߳�cachedģʽ���߳���ֵ
	void setThreadSizeThreshHold(int threshHold);

	//�����̳߳�
	void start(int initThreadSize = std::thread::hardware_concurrency());

	myThreadPool(const myThreadPool&) = delete;
	myThreadPool& operator=(const myThreadPool&) = delete;

	//���pool������״̬
	bool checkRunningState() const;
private:
	//�����̺߳���
	void threadFunc(int threadId);

private:
	//std::vector<std::unique_ptr<Thread>> m_threads;	//�߳��б�
	std::unordered_map <int, std::unique_ptr<myThread>> m_threads;
	//��ʼ���߳�����
	int m_initThreadSize;

	//��¼��ǰ�̳߳������̵߳�������
	std::atomic_int m_curThreadSize;

	//�߳�����������ֵ
	int m_threadSizeThreshHold;

	//��¼�����̵߳�����
	std::atomic_int m_idleThreadSize;

	//�������
	std::queue<std::shared_ptr<ownTask::CTask>> m_taskque;

	//���������
	std::atomic_int m_taskSize;
	//��������������޵���ֵ
	int m_taskqueMaxThresHold;

	//��װ������е��̰߳�ȫ
	std::mutex m_taskQueMtx;

	//��ʾ������в���
	std::condition_variable m_notFull;
	//��ʾ������в���
	std::condition_variable m_notEmpty;

	//��ʾ�ȴ��߳���Դȫ������
	std::condition_variable m_exitCond;

	//��ǰ�̳߳صĹ���ģʽ
	PoolMode m_poolMode;

	//��ʾ��ǰ�̳߳ص�����״̬
	std::atomic_bool m_isPoolRunning;
};

ownThreadPool::myThreadPool& GetThreadPool();
}

#endif // !THREADPOOL_H_