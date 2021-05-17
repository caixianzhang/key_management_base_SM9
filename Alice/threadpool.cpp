#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "threadpool.h"

Task_Node::Task_Node()
{
	pthread_mutex_init(&mutex, NULL);
	arg = NULL;
	next = NULL;
}

Task_Node::~Task_Node()
{
	pthread_mutex_destroy(&mutex);
}

inline void Task_Node::lock()
{
	pthread_mutex_lock(&mutex);
}

inline void Task_Node::unlock()
{
	pthread_mutex_unlock(&mutex);
}

inline void Task_Node::Set_next(Task_Node *next)
{
	this->next = next;
}

inline Task_Node *Task_Node::Get_next()
{
	return next;
}

inline void Task_Node::Set_arg(void *arg)
{
	this->arg = arg;
}

inline void *Task_Node::Get_arg()
{
	return this->arg;
}


Task_Queue::Task_Queue()
{
	pthread_mutex_init(&mutex, NULL);	
	size = 0;
	sem_init(&sem, 0, 0);
	head = rear = NULL;	
}

Task_Queue::~Task_Queue()
{
	pthread_mutex_destroy(&mutex);
	sem_destroy(&sem);
}

inline void Task_Queue::lock()
{
	pthread_mutex_lock(&mutex);
}

inline void Task_Queue::wait()
{
	sem_wait(&sem);
}

inline void Task_Queue::post()
{
	sem_post(&sem);
}

inline void Task_Queue::unlock()
{
	pthread_mutex_unlock(&mutex);
}

inline int Task_Queue::Get_size()
{
	return size;
}

inline void Task_Queue::Inc_size()
{
	size++;
}

inline void Task_Queue::Dec_size()
{
	size--;
}

/*
	向线程池任务队列中加入任务，
	成功：返回0， 
	失败：返回-1，
	本模块不对任务节点内存进行释放，需要有接口使用者自行处理
*/
bool Task_Queue::Add_Task(Task_Node *task)
{
	lock();
	if(Get_size() > Task_Max_Num)
	{
		unlock();
		printf("Task_Queue::Add_Task(Task_Node *Temp_Task):task is too much, please wait......\n");
		return false;
	}else if(Get_size() == 0)
	{
		head = rear = task;
		task->Set_next(NULL);
		Inc_size();
	}else
	{
		task->Set_next(NULL);
		//修改任务队列尾节点的相关属性
		rear->lock();
		rear->Set_next(task);
		rear->unlock();

		rear = task;
		Inc_size();
	}
	
	post();
	unlock();
	return true;
}

/*
	本函数完成从任务队列中取出一个任务节点
*/
Task_Node *Task_Queue::Get_Task()
{
	//等待任务到来
	wait();

	//读取头节点
	Task_Node *Task_To_Execute;
	
	lock();
	Task_To_Execute = head;
	//判断当前任务队列的状态
	if(Get_size() == 1)
	{
		head = rear = NULL;
	}else
	{
		head->lock();
		head = head->Get_next();
		head->unlock();
	}
	//刷新队列大小
	Dec_size();
	unlock();

	return Task_To_Execute;
}


Thread_Node::Thread_Node(Thread_Queue *Idle_Thread_Queue)
{
	//初始化条件变量及锁
	pthread_cond_init(&cond, NULL);
	pthread_mutex_init(&mutex, NULL);

	//获取空闲线程队列地址
	this->Idle_Thread_Queue = Idle_Thread_Queue;
	Task_Flag = false;
	Exit_Flag = false;
	task = NULL;
	next = NULL;
	
	//启动线程
	pthread_create(&Thread_Pid, NULL, Thread_Work, this);
}

//析构函数完成线程退出
Thread_Node::~Thread_Node()
{
	//线程退出标志位置1
	lock();
	Set_Exit_Flag(true);
	//通知线程退出
	signal();
	unlock();

	//等待线程退出，并回收资源
	pthread_join(Thread_Pid, NULL);

	//销毁锁及条件变量
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);
}

inline void Thread_Node::lock()
{
	pthread_mutex_lock(&mutex);
}

inline void Thread_Node::wait()
{
	pthread_cond_wait(&cond, &mutex);
}

inline void Thread_Node::signal()
{
	pthread_cond_signal(&cond);
}

inline void Thread_Node::unlock()
{
	pthread_mutex_unlock(&mutex);
}

inline void Thread_Node::Set_task(Task_Node *task)
{
	if(task == NULL)
	{
		this->task = task;
		Set_Task_Flag(false);
	}else
	{
		lock();
		this->task = task;
		Set_Task_Flag(true);
		signal();
		unlock();
	}
}

inline void Thread_Node::Set_Task_Flag(bool Task_Flag)
{
	this->Task_Flag = Task_Flag;
}

inline bool Thread_Node::Get_Task_Flag()
{
	return Task_Flag;
}

inline void Thread_Node::Set_Exit_Flag(bool Exit_Flag)
{
	this->Exit_Flag = Exit_Flag;
}

inline bool Thread_Node::Get_Exit_Flag()
{
	return Exit_Flag;
}

inline Thread_Queue *Thread_Node::Get_Idle_Thread_Queue()
{
	return Idle_Thread_Queue;
}

inline void Thread_Node::Set_next(Thread_Node *next)
{
	this->next = next;
}

inline Thread_Node *Thread_Node::Get_next()
{
	return next;
}

void *Thread_Node::Thread_Work(void *arg)
{
	Thread_Node *MyThread = (Thread_Node *)arg;
	
	while(1)
	{
		MyThread->lock();
		if(MyThread->Get_Task_Flag() == false)
		{
			MyThread->wait();
			if(MyThread->Get_Exit_Flag() == true)
			{
				MyThread->unlock();
				return (void *)0;
			}
		}

		//执行分配给该线程的任务
		MyThread->task->run();
	

		//任务执行完毕，释放任务节点，重置线程任务
		delete MyThread->task;
	
		MyThread->Set_task(NULL);
		MyThread->Set_next(NULL);
	
		//将本线程加入到空闲线程队列中去
		MyThread->Get_Idle_Thread_Queue()->Add_Thread(MyThread);
	
		//解锁
		MyThread->unlock();
	}
	return (void *)0;
}



Thread_Queue::Thread_Queue()
{
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);

	size = 0;
	head = rear = NULL;
}

Thread_Queue::~Thread_Queue()
{

}

inline void Thread_Queue::lock()
{
	pthread_mutex_lock(&mutex);
}

inline void Thread_Queue::wait()
{
	pthread_cond_wait(&cond, &mutex);	
}

inline void Thread_Queue::signal()
{
	pthread_cond_signal(&cond);
}

inline void Thread_Queue::unlock()
{
	pthread_mutex_unlock(&mutex);
}

inline int Thread_Queue::Get_size()
{	
	return size;
}

inline void Thread_Queue::Inc_size()
{
	size++;
}

inline void Thread_Queue::Dec_size()
{
	size--;
}

//设置头尾节点
inline void Thread_Queue::Set_head(Thread_Node *head)
{
	this->head = head;
}

inline void Thread_Queue::Set_rear(Thread_Node *rear)
{
	this->rear = rear;
}

inline Thread_Node *Thread_Queue::Get_head()
{
	return head;
}

inline Thread_Node *Thread_Queue::Get_rear()
{
	return rear;
}

void Thread_Queue::Add_Thread(Thread_Node *pthread)
{
	//获取当前空闲线程队列的访问锁
	lock();
	//判断当前空闲线程队列的大小
	if(Get_size() == 0)
	{
		head = rear = pthread;
	}else
	{
		rear->lock();
		rear->Set_next(pthread);
		rear->unlock();

		rear = pthread;
	}
	Inc_size();
	signal();
	unlock();
}

//从空闲线程队列中取出一个线程
Thread_Node *Thread_Queue::Get_Thread()
{
	lock();
	if(Get_size() == 0)
	{
		wait();
	}
	Thread_Node *Temp_Thread = Get_head();
	if(Get_size() == 1)
	{
		Set_head(NULL);
		Set_rear(NULL);
	}else
	{
		Temp_Thread->lock();
		Set_head(Temp_Thread->Get_next());
		Temp_Thread->unlock();
	}
	
	Dec_size();
	unlock();

	return Temp_Thread;
}

ThreadPool::ThreadPool()
{
	//初始化空闲线程队列
	Idle_Thread_Queue = new Thread_Queue(); 
	//初始化任务队列
	Ready_Task = new Task_Queue();

	Total_Thread = Thread_Num;
	
	//预制线程池
	for(int i = 0; i < Thread_Num; i++)
	{
		Idle_Thread_Queue->Add_Thread(new Thread_Node(Idle_Thread_Queue));
	}
	
	//开启线程池管理线程
	pthread_create(&Manager_ThreadPool_Pid, NULL, Manager_ThreadPool, this);

	//开启线程池监控线程
	pthread_create(&Monitor_ThreadPool_Pid, NULL, Monitor_ThreadPool, this);
}

ThreadPool::~ThreadPool(){}

inline Thread_Queue *ThreadPool::Get_Idle_Thread_Queue()
{
	return Idle_Thread_Queue;
}

inline Task_Queue *ThreadPool::Get_Ready_Task()
{
	return Ready_Task;
}

inline int ThreadPool::Get_Total_Thread()
{
	return Total_Thread;
}

inline void ThreadPool::Inc_Total_Thread()
{
	Total_Thread++;
}

inline void ThreadPool::Dec_Total_Thread()
{
	Total_Thread--;
}

bool ThreadPool::Add_Task_To_ThreadPool(Task_Node *task)
{
	return Ready_Task->Add_Task(task);
}

void *ThreadPool::Manager_ThreadPool(void *arg)
{
	ThreadPool *MyThreadPool = (ThreadPool *)arg;
	while(1)
	{
		Task_Node *Task_To_Execute = MyThreadPool->Get_Ready_Task()->Get_Task();
		MyThreadPool->Get_Idle_Thread_Queue()->Get_Thread()->Set_task(Task_To_Execute);
	}
	return (void *)0;
}

void *ThreadPool::Monitor_ThreadPool(void *arg)
{
	//ThreadPool *MyThreadPool = (ThreadPool *)arg;
	while(1)
	{
		//printf("Total_Thread:%d, Idle_Thread:%d\n",MyThreadPool->Get_Total_Thread(), MyThreadPool->Get_Idle_Thread_Queue()->Get_size());
		sleep(1);
	}
	return (void *)0;
}

void ThreadPool::Clear_Thread()
{
	for(int i = 0;i < 100; i++)
	{
		delete Idle_Thread_Queue->Get_Thread();
		Dec_Total_Thread();
	}
}

void ThreadPool::Add_Thread()
{
	for(int i = 0; i < 100; i++)
	{
		Idle_Thread_Queue->Add_Thread(new Thread_Node(Idle_Thread_Queue));
		Inc_Total_Thread();
	}
}

