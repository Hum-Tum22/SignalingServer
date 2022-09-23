#include "iThreadPool.h"

#include <thread>
#include <iostream>

const int TASK_MAX_THRESHHOLD = 100000;
const int THREAD_MAX_THRESHHOLD = 1024;
const int THREAD_MAX_IDLE_TIME = 60;	//��λ����

/*
-------------------------------
�̷߳�����ʵ��
-------------------------------
*/
using namespace ownThreadPool;
int myThread::m_generateId = 0;

myThread::myThread(ThreadFunc func)
	:m_func(func)
	, m_threadId(m_generateId++)
{

}
// �߳�����
myThread::~myThread()
{

}

void myThread::start()
{
	//����һ���߳���ִ��һ���̺߳���
	std::thread t(m_func, m_threadId);	//C++11�̶߳�����̺߳���
	t.detach();	//�����ػ��߳�
}

int myThread::getId() const
{
	return m_threadId;
}

/*
-------------------------------
�̳߳ط�����ʵ��
-------------------------------
*/
//�̹߳���
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

//�̳߳�����
myThreadPool::~myThreadPool()
{
	m_isPoolRunning = false;

	//�ȴ��̳߳��������е��̷߳��� 
	std::unique_lock<std::mutex> lock(m_taskQueMtx);
	m_notEmpty.notify_all();
	m_exitCond.wait(lock, [&]()->bool {return m_threads.size() == 0; });
}

//�����̳߳صĹ���ģʽ
void myThreadPool::setMode(PoolMode mode)
{
	if (checkRunningState()) return;
	m_poolMode = mode;
}

//����task����������ߵ���ֵ
void myThreadPool::setTaskQueMaxThrshHold(int threshhold)
{
	m_taskqueMaxThresHold = threshhold;
}

//���̳߳��ύ����
bool myThreadPool::submitTask(std::shared_ptr<ownTask::CTask> sp)
{
	//��ȡ��
	std::unique_lock<std::mutex> lock(m_taskQueMtx);

	//�̵߳�ͨ�� �ȴ������п���
	//�û��ύ���� 
	//m_notFull.wait(lock, [&]()->bool {return m_taskque.size() < TASK_MAX_THRESHHOLD; });

	//��������ܳ���1s �����߳��ύʧ��
	if (!m_notFull.wait_for(lock, std::chrono::seconds(1),[&]()->bool {return m_taskque.size() < TASK_MAX_THRESHHOLD; }))
	{
		std::cerr << "task queue is full, submit task failed." << std::endl;
		return false;// Result(sp, false);
	}

	//����п��࣬��ô�÷������������
	m_taskque.emplace(sp);
	m_taskSize++;

	//��Ϊ�·������� ��ô����Ҳ�Ͳ�Ϊ���ˣ���not_empty�Ͻ���֪ͨ ���̸߳Ͽ�ִ������
	m_notEmpty.notify_one();

	//cached ������ȽϽ��� ������С���������
	//��Ҫ�������������Ϳ����̵߳��������ж��Ƿ���Ҫ�ж��µ��߳�
	if (m_poolMode == PoolMode::MODE_CACHED &&
		m_taskSize > m_idleThreadSize &&
		m_curThreadSize < m_threadSizeThreshHold)
	{
		std::cout << ">>>create new thread..." << std::endl;
		//�������߳�
		auto ptr = std::unique_ptr<myThread>(new myThread(std::bind(&myThreadPool::threadFunc, this, std::placeholders::_1)));
		//m_threads.emplace_back(std::move(ptr));
		int threadId = ptr->getId();
		m_threads.emplace(threadId, std::move(ptr));
		//�����߳�
		m_threads[threadId]->start();
		//�޸��̸߳�����صı���
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

//�����̳߳�
void myThreadPool::start(int initThreadSize)
{
	//�����̳߳ص�����״̬
	m_isPoolRunning = true;
	//��¼��ʼ�̸߳���
	m_initThreadSize = initThreadSize;
	m_curThreadSize = initThreadSize;

	//�����̶߳���
	for (int i = 0; i < m_initThreadSize; i++)
	{
		//����thread�̶߳����ʱ��
		//���̺߳�������thread�̶߳���
		//m_threads.emplace_back(new Thread(std::bind(&myThreadPool::threadFunc, this)));
		auto ptr = std::unique_ptr<myThread>(new myThread(std::bind(&myThreadPool::threadFunc, this, std::placeholders::_1)));
		//m_threads.emplace_back(std::move(ptr));
		int threadId = ptr->getId();
		m_threads.emplace(threadId, std::move(ptr));
	}

	//���������߳� std::vector<Thread*> m_threads;
	for (int i = 0; i < m_initThreadSize; i++)
	{
		m_threads[i]->start();  //��Ҫȥִ��һ���̺߳���
		m_idleThreadSize++;		//��¼��ʼ�����̵߳�����
	}
}

//�����̺߳���
void myThreadPool::threadFunc(int threadId)
{
	auto lastTime = std::chrono::high_resolution_clock().now();
	
	//m_threads[threadId]->
	//m_ThreadState = _t_starte;
	while (true)
	{
		std::shared_ptr<ownTask::CTask> task;
		{
			//��ȡ��
			std::unique_lock<std::mutex> lock(m_taskQueMtx);

			std::cout << "tid:" << std::this_thread::get_id() << "���ڳ��Ի�ȡ����..." << std::endl;

			//cachedģʽ�� �п����Ѿ������˺ܶ���߳� ���ǿ���ʱ�䳬��60s
			//Ӧ�ðѶ�����̻߳��ս�����������m_initThreadSize�������߳̽��л��գ�
			//��ǰʱ�� - ��һ���߳�ִ�е��¼� > 60s

			// ÿһ���ӷ���һ�� ���֣���ʱ���غ��������ִ�з���
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
							//���յ�ǰ�߳�
							//��¼�߳���������ر���ֵ���޸�
							//���̶߳�����߳��б�������ɾ�� û�а취ƥ��threadFunc��Ӧ������һ���̶߳���
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
					//�ȴ�notEmpty����
					m_notEmpty.wait(lock);
				}
				//�̳߳�Ҫ���� �����߳���Դ	

			}
			m_idleThreadSize--;

			std::cout << "tid:" << std::this_thread::get_id() << "��ȡ����ɹ�" << std::endl;

			//����������л�ȡһ������
			task = m_taskque.front();
			m_taskque.pop();
			m_taskSize--;

			//�����Ȼ���������� ��֪ͨ�����߳�ִ������
			if (m_taskque.size() > 0)
			{
				m_notEmpty.notify_one();
			}

			//ȡ��һ���������֪ͨ��֪ͨ���Լ����ύ����
			m_notFull.notify_all();
		}//��ʱ�ͷ���
		if (task != nullptr)
		{
			task->TaskRun();
			task->TaskClose();
			task->TaskDestory();
		}
		m_idleThreadSize++;
		//�����߳�ִ����������ʱ��
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