#ifndef RAND_H_
#define RAND_H_

#include <semaphore.h>

class Rand256_Node{
	private:
		unsigned char Rand256[32];
		Rand256_Node *next;
	public:
		Rand256_Node(unsigned char *TempRand256);
		
		virtual ~Rand256_Node();
		
		//设置后向节点
		inline void Set_next(Rand256_Node *next);
		
		//读取后向节点
		inline Rand256_Node *Get_next();

		//读取随机数
		unsigned char *Get_Rand256();
};

class Rand256_Pool{
	private:
		//访问随机数池锁
		pthread_mutex_t mutex;
		enum Queue_Size{Rand256_Max_Num = 100};
		//当前随机数池大小
		int size;
		//随机数池信号量
		sem_t sem;
		//随机数池头尾节点
		Rand256_Node *head;
		Rand256_Node *rear;

		//硬件随机设备文件描述符
		int dev_random_fd;
		
		//监控随机数池大小线程
		pthread_t Monitor_Rand256_Pool_Pid;
	 	
		static void *Monitor_Rand256_Pool(void *arg);
	public:
		Rand256_Pool();
		virtual ~Rand256_Pool();

		inline int Get_dev_random_fd();

		inline void lock();
		inline void wait();
		inline void post();
		inline void unlock();

		//向池中添加随机数节点
		void Add_Rand256_Node(Rand256_Node *New_Rand256_Node);
		
		//读取一个随机数节点
		Rand256_Node *Get_Rand256_Node();
		
		//获取随机数池大小
		inline int Get_size();
};

#endif
