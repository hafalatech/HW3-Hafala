#include "threadPool.h"


void executer(ThreadPool* pool) { 
	while(pool->has_destroyed == 0 || pool->shouldWaitForTasks != 0) {  
		
		pthread_mutex_lock(pool->mutexForCond);
		while(osIsQueueEmpty(pool->tasks_queue)) {
			pthread_cond_wait(pool->cond, pool->mutexForCond);
		}
		pthread_mutex_unlock(pool->mutexForCond);
		
		int res = pthread_mutex_trylock(pool->mutex);
		if(res != 0) {
			continue;
		}

		if(pool->shouldWaitForTasks != 0)
		{
			TaskNode node = osDequeue(pool->tasks_queue);
			pthread_mutex_unlock(pool->mutex);
			(*node.routine)(node.param);
			continue;
		}
		pthread_mutex_unlock(pool->mutex);
	}
}


ThreadPool* tpCreate(int numOfThreads) {
	ThreadPool* new_pool = (ThreadPool*)malloc(sizeof(ThreadPool));
	new_pool->tasks_queue = osCreateQueue();
	new_pool->threads =  (pthread_t*)malloc(numOfThreads * sizeof(pthread_t));
	new_pool->has_destroyed = 0;
	new_pool->shouldWaitForTasks = 0;
	pthread_mutex_init(new_pool->mutex , PTHREAD_MUTEX_ERRORCHECK);
	pthread_cond_init(new_pool->cond, NULL);
	pthread_mutex_init(new_pool->mutexForCond , PTHREAD_MUTEX_ERRORCHECK);
	int i;
	for(i = 0 ; i < numOfThreads ; i++){
		pthread_create(threads + i, NULL, &executer, new_pool);		
	}
	return new_pool;
}


void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {
	threadPool->has_destroyed = 1;
	threadPool->shouldWaitForTasks = shouldWaitForTasks;
	


	if (shouldWaitForTasks != 0)
	{
		pthread_cond_t* condForWait;
		pthread_mutex_t* mutexForWait;
		pthread_cond_init(condForWait, NULL);
		pthread_mutex_init(mutexForWait, PTHREAD_MUTEX_ERRORCHECK);
		
		while(!osIsQueueEmpty(pool->tasks_queue)) {
			pthread_cond_wait(condForWait, mutexForWait);
		}

		pthread_cond_destroy(condForWait);
		pthread_mutex_destroy(mutexForWait);
	}

	pthread_cond_broadcast(threadPool->cond); 	// WAIT FOR THREADS

	while(!osIsQueueEmpty(threadPool->tasks_queue)) {
		TaskNode* current = osDequeue(threadPool->tasks_queue);
		free(current);
	}
	osDestroyQueue(threadPool->tasks_queue);
	for(i = 0 ; i < numOfThreads ; i++){
		pthread_exit(threadPool->threads + i);		
	}
	free(threadPool->threads);
	pthread_mutex_destroy(threadPool->mutex); //what if fails?
	pthread_cond_destroy(threadPool->cond);
	pthread_mutex_destroy(threadPool->mutex); //what if fails?
	free(threadPool);
}


int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param) {
	int res = -1;
	if(threadPool->shouldWaitForTasks != 0) 
	{
		return res;
	}
	res = 0;
	task_node* newTask = (task_node*)malloc(sizeof(task_node));
	newTask->param = param;
	newTask->routine = computeFunc;

	osEnqueue(threadPool->tasks_queue, newTask);
	return res;	
}





