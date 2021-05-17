#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <semaphore.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#include "rand.h"

Rand256_Node::Rand256_Node(unsigned char *Rand256)
{
	memcpy(this->Rand256, Rand256, 32);
	next = NULL;
}
		
Rand256_Node::~Rand256_Node()
{
	
}

inline void Rand256_Node::Set_next(Rand256_Node *next)
{
	this->next = next;
}

inline Rand256_Node *Rand256_Node::Get_next()
{
	return next;
}

unsigned char *Rand256_Node::Get_Rand256()
{
	return Rand256;
}


Rand256_Pool::Rand256_Pool()
{
	pthread_mutex_init(&mutex, NULL);
	size = 0;
	sem_init(&sem, 0, 0);
	head = rear = 0;
	/*
		注意:这里如果考虑严格意义上的随机数，应该打开/dev/random 这个设备，
		当系统内随机情况不多时，会出现read()函数阻塞，
		如果考虑速度的情况下，可以考虑打开/dev/urandom 这个设备，它不会使read()函数
		发生阻塞，但随机性会下降
	*/
	dev_random_fd = open("/dev/urandom", O_RDONLY);
	if(dev_random_fd == -1)
	{
		printf("Rand256_Pool::Rand256_Pool(): fail to open rand dev !!! %m\n");
	}
	//创建随机数池监控线程
	pthread_create(&Monitor_Rand256_Pool_Pid, NULL, Monitor_Rand256_Pool, this);
}

Rand256_Pool::~Rand256_Pool(){}

void *Rand256_Pool::Monitor_Rand256_Pool(void *arg)
{
	Rand256_Pool *My_Rand256_Pool = (Rand256_Pool *)arg;
	unsigned char Rand256[32];
	while(1)
	{
		for(int i = 0;i < 32; i++)
		{
			if(read(My_Rand256_Pool->Get_dev_random_fd(), Rand256 + i, 1) == -1)
			{
				printf("Rand256_Pool::Add_Rand256_Node:fail to get rand num !!! %m\n");
			}
		}
		
		Rand256_Node *New_Rand256_Node = new Rand256_Node(Rand256);
		
		My_Rand256_Pool->lock();
		if(My_Rand256_Pool->Get_size() > Rand256_Max_Num)
		{
			My_Rand256_Pool->unlock();
			sleep(1);
			continue;
		}else
		{
			My_Rand256_Pool->Add_Rand256_Node(New_Rand256_Node);
			My_Rand256_Pool->unlock();
			My_Rand256_Pool->post();
		}
	}
}

inline int Rand256_Pool::Get_dev_random_fd()
{
	return dev_random_fd;
}

inline void Rand256_Pool::lock()
{
	pthread_mutex_lock(&mutex);
}

inline void Rand256_Pool::wait()
{
	sem_wait(&sem);
}

inline void Rand256_Pool::post()
{
	sem_post(&sem);
}

inline void Rand256_Pool::unlock()
{
	pthread_mutex_unlock(&mutex);
}


void Rand256_Pool::Add_Rand256_Node(Rand256_Node *New_Rand256_Node)
{
	if(size == 0)
	{
		head = rear = New_Rand256_Node;
	}else
	{
		rear->Set_next(New_Rand256_Node);
		rear = rear->Get_next();
	}
	size++;
}

Rand256_Node *Rand256_Pool::Get_Rand256_Node()
{
	wait();
	lock();
	Rand256_Node *New_Rand_256 = head;
	head = head->Get_next();
	size--;
	unlock();
	return New_Rand_256;
}

inline int Rand256_Pool::Get_size()
{
	return size;
}