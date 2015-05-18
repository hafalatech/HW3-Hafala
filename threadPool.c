#include "threadPool.h"


void executer(ThreadPool* pool) { 
	while(pool->has_destroyed == 0 || pool->shouldWaitForTasks == 0) {  
		while(osIsQueueEmpty(pool->tasks_queue)) {
			pthread_cond_wait(&cond, pool->mutex);	//TODO: Check mutex
		}
		int res = pthread_mutex_trylock(pool->mutex);
		if(res != 0) {
			continue;
		}
		TaskNode node = osDequeue(pool->tasks_queue);
		pthread_mutex_unlock(pool->mutex);
		(*node.routine)(node.param);
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
	int i;
	for(i = 0 ; i < numOfThreads ; i++){
		pthread_create(threads + i, NULL, &executer, new_pool);		
	}
	return new_pool;
}


void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {
	threadPool->has_destroyed = 1;
	threadPool->shouldWaitForTasks = shouldWaitForTasks;
	
	//TOD0: WAIT FOR THREADS
	
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
	free(threadPool);
}


int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param) {
	if(shouldWaitForTasks != 0); //no insert



}