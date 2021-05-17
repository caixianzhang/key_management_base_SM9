#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <semaphore.h>

//任务节点
class Task_Node{
	private:
		//访问该任务节点的锁
		pthread_mutex_t mutex;

		//具体要执行的任务的参数，任务执行完毕后，参数内存由调用者释放
		void *arg;

		//该任务在队列中的下一个节点
		Task_Node *next;

	public:
		Task_Node();
		virtual ~Task_Node();

		inline void lock();
		inline void unlock();
		
		inline void Set_next(Task_Node *next);
		inline Task_Node *Get_next();
		inline void Set_arg(void *arg);
		inline void *Get_arg();
		virtual void run() = 0;
};
	
//任务队列
class Task_Queue{
	private:
		//访问任务队列锁
		pthread_mutex_t mutex;
		enum Queue_Size{Task_Max_Num = 1000};
		//任务队列大小
		int size;
		//任务队列信号量
		sem_t sem;
		//任务队列头尾节点
		Task_Node *head;
		Task_Node *rear;	

	public:
		Task_Queue();
		virtual ~Task_Queue();

		inline void lock();
		inline void wait();
		inline void post();
		inline void unlock();

		//向任务队列中添加任务
		bool Add_Task(Task_Node *task);
		
		//从任务队列中取出任务
		Task_Node *Get_Task();

		//获取队列大小
		inline int Get_size();

		//队列大小加一
		inline void Inc_size();
		//队列大小减一
		inline void Dec_size();
};

class Thread_Queue;
/*
	线程节点
*/
class Thread_Node{
	private:
		//单个线程唤醒条件变量
		pthread_cond_t cond;

		//修改单个线程属性的线程锁
		pthread_mutex_t mutex;

		//单个线程的线程ID
		pthread_t Thread_Pid;

		//是否有任务执行
		bool Task_Flag;
		
		//任务执行完毕后是否退出
		bool Exit_Flag;

		//分配给该线程的任务
		Task_Node *task;

		//空闲线程队列指针
		Thread_Queue *Idle_Thread_Queue;
			
		//在线程队列中的下一个线程
		Thread_Node *next;

		//单个线程实际开始工作
		static void *Thread_Work(void *arg);
	public:
		Thread_Node(Thread_Queue *Idle_Thread_Queue);
		virtual ~Thread_Node();	

		inline void Start();
		inline void lock();
		inline void wait();
		inline void signal();
		inline void unlock();

		//设置该线程节点需要执行的任务
		inline void Set_task(Task_Node *task);

		//设置线程有任务执行标志
		inline void Set_Task_Flag(bool Task_Flag);
		//读取线程是否有任务执行标志
		inline bool Get_Task_Flag();

		//设置线程退出标志
		inline void Set_Exit_Flag(bool Exit_Flag);
		//读取线程退出标志
		inline bool Get_Exit_Flag();

		//获取空闲线程队列
		inline Thread_Queue *Get_Idle_Thread_Queue();
		
		//设置线程节点后向指针
		inline void Set_next(Thread_Node *next);
		inline Thread_Node *Get_next();
};	

//线程队列(主要针对空闲线程队列)
class Thread_Queue{
	private:
		//访问修改该线程队列的锁
		pthread_mutex_t mutex;

		//线程队列唤醒条件变量
		pthread_cond_t cond;

		//线程队列大小
		int size;

		//线程队列头尾节点
		Thread_Node *head;
		Thread_Node *rear;
	public:
		Thread_Queue();
		virtual ~Thread_Queue();

		inline void lock();
		inline void wait();
		inline void signal();
		inline void unlock();

		//获取队列大小
		inline int Get_size();
		//队列大小加一
		inline void Inc_size();
		//队列大小减一
		inline void Dec_size();

		//设置头尾节点
		inline void Set_head(Thread_Node *head);
		inline void Set_rear(Thread_Node *rear);

		//获取头尾节点
		inline Thread_Node *Get_head();
		inline Thread_Node *Get_rear();

		//任务执行完毕，需要将线程加入到空闲线程队列中去
		void Add_Thread(Thread_Node *pthread);

		//从空闲线程队列中取出一个线程
		Thread_Node *Get_Thread();
};


//线程池总模块
class ThreadPool{
	private:
		//设置预制线程数大小为200
		enum Thread_Num{Thread_Num = 200};

		//线程池中所有线程的数量
		int Total_Thread;
		
		//空闲线程队列
		Thread_Queue *Idle_Thread_Queue;

		//任务队列
		Task_Queue	*Ready_Task;

		//线程池管理线程
		pthread_t Manager_ThreadPool_Pid;

		//线程池监控线程
		pthread_t Monitor_ThreadPool_Pid;

		//线程池管理线程,本线程完成将任务队列中的任务加入到线程池中
		static void *Manager_ThreadPool(void *arg);

		//线程池监控线程，本线程完成监控空闲线程大小，根据情况增减空闲线程
		static void *Monitor_ThreadPool(void *arg);
	public:
		ThreadPool();
		virtual ~ThreadPool();
		inline Thread_Queue *Get_Idle_Thread_Queue();
		inline Task_Queue *Get_Ready_Task();
		
		inline int Get_Total_Thread();
		inline void Inc_Total_Thread();
		inline void Dec_Total_Thread();

		void Clear_Thread();
		void Add_Thread();

		//向线程池中加入任务
		bool Add_Task_To_ThreadPool(Task_Node *task);
};

#endif
