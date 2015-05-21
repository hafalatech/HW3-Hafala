#include "threadPool.h"


void* executer(void* arg) { 
	ThreadPool* pool = (ThreadPool*)(arg);
	while(pool->has_destroyed == 0 || pool->shouldWaitForTasks != 0) {  
		
		pthread_mutex_lock(&(pool->mutexForCond));
		while(osIsQueueEmpty(pool->tasks_queue)) {
			pthread_cond_wait(&(pool->cond), &(pool->mutexForCond));
		}
		pthread_mutex_unlock(&(pool->mutexForCond));
		
		int res = pthread_mutex_trylock(&(pool->mutex));
		if(res != 0) {
			continue;
		}

		if(pool->shouldWaitForTasks != 0)
		{
			TaskNode* node = osDequeue(pool->tasks_queue);
			pool->runningTasks++;
			pthread_mutex_unlock(&(pool->mutex));
			(*node->routine)(node->param);

			pthread_mutex_lock(&(pool->mutex));
			pool->runningTasks--; //todo lock this
			pthread_mutex_unlock(&(pool->mutex));
			continue;
		}
		pthread_mutex_unlock(&(pool->mutex));
	}
	return NULL;
}


ThreadPool* tpCreate(int numOfThreads) {
	pthread_mutexattr_t attribute;
	pthread_mutexattr_settype(&attribute, PTHREAD_MUTEX_ERRORCHECK);
	ThreadPool* new_pool = (ThreadPool*)malloc(sizeof(ThreadPool));
	new_pool->tasks_queue = osCreateQueue();
	new_pool->threads =  (pthread_t*)malloc(numOfThreads * sizeof(pthread_t));
	new_pool->has_destroyed = 0;
	new_pool->shouldWaitForTasks = 0;

	pthread_mutex_init(&(new_pool->mutex) , &attribute);
	pthread_cond_init(&(new_pool->cond), NULL);
	pthread_mutex_init(&(new_pool->mutexForCond) , &attribute);
	new_pool->numOfThreads = numOfThreads;
	new_pool->runningTasks = 0;
	int i;
	for(i = 0 ; i < numOfThreads ; i++){
		pthread_create(new_pool->threads + i, NULL, executer, new_pool);		
	}
	return new_pool;
}


void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {
	threadPool->has_destroyed = 1;
	threadPool->shouldWaitForTasks = shouldWaitForTasks;
	
	if (shouldWaitForTasks != 0)
	{
		pthread_mutexattr_t attribute;
		pthread_mutexattr_settype(&attribute, PTHREAD_MUTEX_ERRORCHECK);
		pthread_cond_t condForWait;
		pthread_mutex_t mutexForWait;
		pthread_cond_init(&condForWait, NULL);
		pthread_mutex_init(&mutexForWait, &attribute);
		
		while(!osIsQueueEmpty(threadPool->tasks_queue)) {
			pthread_cond_wait(&condForWait, &mutexForWait);
		}

		pthread_cond_destroy(&condForWait);
		pthread_mutex_destroy(&mutexForWait);
	}


	//make sure all threads are in busy
	pthread_mutexattr_t attribute2;
	pthread_mutexattr_settype(&attribute2, PTHREAD_MUTEX_ERRORCHECK);
	pthread_cond_t condForWait2;
	pthread_mutex_t mutexForWait2;
	pthread_cond_init(&condForWait2, NULL);
	pthread_mutex_init(&mutexForWait2, &attribute2);
	
	while(threadPool->runningTasks != 0) {
		pthread_cond_wait(&condForWait2, &mutexForWait2);
	}

	pthread_cond_destroy(&condForWait2);
	pthread_mutex_destroy(&mutexForWait2);
	//got out of here when all threads are stuck in busy








	pthread_cond_broadcast(&(threadPool->cond)); 	// WAIT FOR THREADS
	while(!osIsQueueEmpty(threadPool->tasks_queue)) {
		TaskNode* current = osDequeue(threadPool->tasks_queue);
		free(current);
	}
	osDestroyQueue(threadPool->tasks_queue);
	int i;
	for(i = 0 ; i < threadPool->numOfThreads ; i++){
		//pthread_cancel(threadPool->threads[i]);
		pthread_join(threadPool->threads[i], NULL);
		//pthread_exit(threadPool->threads + i);		
	}

	pthread_mutex_destroy(&(threadPool->mutex)); //what if fails?
	pthread_cond_destroy(&(threadPool->cond));
	pthread_mutex_destroy(&(threadPool->mutex)); //what if fails?
	free(threadPool->threads);
	free(threadPool);
}


int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param) {
	int res = -1;
	if(threadPool->shouldWaitForTasks != 0) 
	{
		return res;
	}
	res = 0;
	TaskNode* newTask = (TaskNode*)malloc(sizeof(TaskNode));
	newTask->param = param;
	newTask->routine = computeFunc;

	osEnqueue(threadPool->tasks_queue, newTask);
	return res;	
}





