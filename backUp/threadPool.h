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
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	pthread_mutex_t mutexForCond;
	int numOfThreads;
	int runningTasks;
}ThreadPool;


ThreadPool* tpCreate(int numOfThreads);


void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);


int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

#endif
