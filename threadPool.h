#ifndef __THREAD_POOL__
#define __THREAD_POOL__
#include <stdlib.h>
#include <stdio.h>              //TODO - remove this include before submission
#include <pthread.h>
#include "osqueue.h"

//Task info
typedef struct task_node
{   
	void* param;
	void (*routine) (void*);
}TaskNode;


typedef struct thread_pool
{
	OSQueue* tasks_queue;
	pthread_t* threads;
	int has_destroyed;
	int shouldWaitForTasks;
	pthread_mutex_t mutexForExecuter;
	
	pthread_cond_t executerCond;
	pthread_mutex_t executerCondMutex;

	pthread_cond_t destroyCond;
	pthread_mutex_t destroyCondMutex;

	int numOfThreads;
	int runningTasks;
	int threadsAlive;
}ThreadPool;


ThreadPool* tpCreate(int numOfThreads);


void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);


int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

#endif
