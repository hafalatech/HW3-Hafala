#include "threadPool.h"


void* executer(void* arg) {
	ThreadPool* pool = (ThreadPool*)(arg);
	while(pool->has_destroyed == 0 || pool->shouldWaitForTasks != 0) {  		
		pthread_mutex_lock(&(pool->mutexForExecuter));
			if((pool->has_destroyed != 0 && pool->shouldWaitForTasks == 0) || (pool->has_destroyed != 0 && osIsQueueEmpty(pool->tasks_queue)))
			{		
				pthread_mutex_unlock(&(pool->mutexForExecuter));
				pthread_cond_signal(&(pool->destroyCond));
				return NULL;
			}
			pthread_mutex_lock(&(pool->executerCondMutex));
			while(osIsQueueEmpty(pool->tasks_queue)) {
				if (pool->has_destroyed == 1)
				{
					pthread_cond_signal(&(pool->destroyCond));
				}
				pthread_cond_wait(&(pool->executerCond), &(pool->executerCondMutex));
				if((pool->has_destroyed != 0 && pool->shouldWaitForTasks == 0) || (pool->has_destroyed != 0 && osIsQueueEmpty(pool->tasks_queue)))
				{	
					pthread_mutex_unlock(&(pool->executerCondMutex));	
					pthread_mutex_unlock(&(pool->mutexForExecuter));		
					pthread_cond_signal(&(pool->destroyCond));
					return NULL;
				}
			}
			pthread_mutex_unlock(&(pool->executerCondMutex));	
			if(pool->has_destroyed == 0 || pool->shouldWaitForTasks != 0)
			{
				TaskNode* node = osDequeue(pool->tasks_queue);
				pthread_mutex_unlock(&(pool->mutexForExecuter));
				(*node->routine)(node->param);
				pthread_mutex_lock(&(pool->mutexForExecuter));
				free(node);
				pthread_mutex_unlock(&(pool->mutexForExecuter));
				if (pool->has_destroyed!=0) {
					pthread_cond_signal(&(pool->destroyCond));	//wakeup the destroy in case it was called
				}			
				continue;
			}
		pthread_mutex_unlock(&(pool->mutexForExecuter));
	}

	pthread_cond_signal(&(pool->destroyCond));	//wakeup the destroy in case it was called
	return NULL;
}


ThreadPool* tpCreate(int numOfThreads) {
	ThreadPool* new_pool = (ThreadPool*)malloc(sizeof(ThreadPool));
	new_pool->tasks_queue = osCreateQueue();
	new_pool->threads =  (pthread_t*)malloc(numOfThreads * sizeof(pthread_t));
	new_pool->has_destroyed = 0;
	new_pool->shouldWaitForTasks = 0;

	pthread_mutex_init(&(new_pool->mutexForExecuter) , NULL);
	
	pthread_cond_init(&(new_pool->executerCond), NULL);
	pthread_mutex_init(&(new_pool->executerCondMutex) , NULL);
	
	pthread_cond_init(&(new_pool->destroyCond), NULL);
	pthread_mutex_init(&(new_pool->destroyCondMutex) , NULL);
	
	new_pool->numOfThreads = numOfThreads;
	int i;
	for(i = 0 ; i < numOfThreads ; i++) {
		pthread_create(new_pool->threads + i, NULL, executer, new_pool);
	}
	return new_pool;
}


void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {
	if (threadPool->has_destroyed==1) {
		return;
	}
	threadPool->has_destroyed = 1;
	threadPool->shouldWaitForTasks = shouldWaitForTasks;

	if (shouldWaitForTasks != 0) {
		pthread_mutex_lock(&(threadPool->destroyCondMutex));
		while(!osIsQueueEmpty(threadPool->tasks_queue)) {
			pthread_cond_broadcast(&(threadPool->executerCond));
			pthread_cond_wait(&threadPool->destroyCond, &threadPool->destroyCondMutex);
		}
		pthread_mutex_unlock(&(threadPool->destroyCondMutex));
	}

	threadPool->shouldWaitForTasks = 0;
	
	pthread_cond_broadcast(&(threadPool->executerCond));

	//free tasks
	while(!osIsQueueEmpty(threadPool->tasks_queue)) {
		TaskNode* current = osDequeue(threadPool->tasks_queue);
		free(current);
	}
	//free queue
	osDestroyQueue(threadPool->tasks_queue);
	int i;
	//join threads
	for(i = 0 ; i < threadPool->numOfThreads ; i++){
		pthread_join(threadPool->threads[i], NULL);	
	}
	//free memory allocated
	pthread_mutex_destroy(&(threadPool->mutexForExecuter));
	pthread_mutex_destroy(&threadPool->destroyCondMutex);
	pthread_mutex_destroy(&threadPool->executerCondMutex);
	pthread_cond_destroy(&(threadPool->executerCond));
	pthread_cond_destroy(&threadPool->destroyCond);
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
	if(threadPool->has_destroyed != 0 && threadPool->shouldWaitForTasks == 0) 
	{
		return res;
	}
	TaskNode* newTask = (TaskNode*)malloc(sizeof(TaskNode));
	newTask->param = param;
	newTask->routine = computeFunc;
	osEnqueue(threadPool->tasks_queue, newTask);
	pthread_cond_broadcast(&(threadPool->executerCond));	//free the threads to do some work!
	return res;	
}





