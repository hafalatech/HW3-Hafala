#include "threadPool.h"




ThreadPool* tpCreate(int numOfThreads) {
	ThreadPool* new_pool = (ThreadPool*)malloc(sizeof(ThreadPool));
	new_pool->tasks_queue = osCreateQueue();
	new_pool->threads =  (pthread_t*)malloc(numOfThreads * sizeof(pthread_t));
	int i;
	for(i = 0 ; i < numOfThreads ; i++){
		pthread_create(threads + i, NULL, );		//TODO: Complete
	}
	return new_pool;
}


void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {
	if(shouldWaitForTasks == 0){
		
	} else { 
	
	}
}


int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param) {




}