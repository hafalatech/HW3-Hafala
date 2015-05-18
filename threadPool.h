#ifndef __THREAD_POOL__
#define __THREAD_POOL__
#include <pthread.h>
#include "osqueue.h"

//Task info
typedef struct task_node
{   
	void* param;
	void* (*routine) (void*);
}TaskNode;


typedef struct thread_pool
{
	OSQueue* tasks_queue;	// Queue containing all the tasks in the pool   
	pthread_t* threads;            // Array containing all threads in the pool
	int has_destroyed;
	int shouldWaitForTasks;
	pthread_mutex_t* mutex;
	pthread_cond_t* cond;  
}ThreadPool;


ThreadPool* tpCreate(int numOfThreads);


void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);


int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

#endif
