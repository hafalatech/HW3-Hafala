#include "threadPool.h"


void* executer(void* arg) {
	printf("[THREAD] - IM ALIVE\n");
	ThreadPool* pool = (ThreadPool*)(arg);
	pthread_mutex_lock(&(pool->mutexForExecuter));
	int myIndex = pool->threadsAlive;
	printf("[THREAD %d] - IM ALIVE\n",myIndex);
	pthread_mutex_unlock(&(pool->mutexForExecuter));
	while(pool->has_destroyed == 0 || pool->shouldWaitForTasks != 0) {  		
		pthread_mutex_lock(&(pool->mutexForExecuter));
			if((pool->has_destroyed != 0 && pool->shouldWaitForTasks == 0) || (pool->has_destroyed != 0 && osIsQueueEmpty(pool->tasks_queue)))
			{		
				printf("[THREAD %d] - WAKING UP DESTROY 1\n",myIndex);		
				pthread_cond_signal(&(pool->destroyCond)); //wakeup the destroy in case it was called
				pool->threadsAlive--;
		pthread_mutex_unlock(&(pool->mutexForExecuter));
				printf("[THREAD %d] - IM DEAD\n",myIndex);
				return NULL;
			}
			while(osIsQueueEmpty(pool->tasks_queue)) {
				printf("[THREAD %d] - WAKING UP DESTROY 2\n",myIndex);
				pthread_cond_signal(&(pool->destroyCond)); //wakeup the destroy in case it was called

				printf("[THREAD %d] - GOING TO SLEEP\n",myIndex);
				pthread_cond_wait(&(pool->executerCond), &(pool->executerCondMutex));
				printf("[THREAD %d] - WOKE UP\n",myIndex);			
			}	

			if(pool->has_destroyed == 0 || pool->shouldWaitForTasks != 0)
			{
				printf("[THREAD %d] - TAKING TASK FROM QUEUE\n",myIndex);
				TaskNode* node = osDequeue(pool->tasks_queue);
				pool->runningTasks++;
		pthread_mutex_unlock(&(pool->mutexForExecuter));

				(*node->routine)(node->param);

				printf("[THREAD %d] - FINISHED TASK\n",myIndex);
				pthread_mutex_lock(&(pool->mutexForExecuter));
					printf("[THREAD %d] - DEC runningTasks--\n",myIndex);
					pool->runningTasks--;
				pthread_mutex_unlock(&(pool->mutexForExecuter));
				if (pool->has_destroyed!=0){
					printf("[THREAD %d] - WAKING UP DESTROY 3\n",myIndex);
					pthread_cond_signal(&(pool->destroyCond)); //wakeup the destroy in case it was called
				}			
				continue;
			}
		pthread_mutex_unlock(&(pool->mutexForExecuter));
	}

	pthread_mutex_lock(&(pool->mutexForExecuter));
		pool->threadsAlive--;
	pthread_mutex_unlock(&(pool->mutexForExecuter));
	printf("[THREAD %d] - WAKING UP DESTROY 4\n",myIndex);
	pthread_cond_signal(&(pool->destroyCond)); //wakeup the destroy in case it was called
	printf("[THREAD %d] - IM DEAD\n",myIndex);	
	return NULL;
}


ThreadPool* tpCreate(int numOfThreads) {
	ThreadPool* new_pool = (ThreadPool*)malloc(sizeof(ThreadPool));
	new_pool->tasks_queue = osCreateQueue();
	new_pool->threads =  (pthread_t*)malloc(numOfThreads * sizeof(pthread_t));
	new_pool->has_destroyed = 0;
	new_pool->shouldWaitForTasks = 0;
	new_pool->threadsAlive = 0;

	pthread_mutex_init(&(new_pool->mutexForExecuter) , NULL);
	pthread_cond_init(&(new_pool->executerCond), NULL);

	pthread_cond_init(&(new_pool->destroyCond), NULL);
	pthread_mutex_init(&(new_pool->destroyCondMutex) , NULL);
	pthread_mutex_init(&(new_pool->executerCondMutex) , NULL);

	new_pool->numOfThreads = numOfThreads;
	new_pool->runningTasks = 0;
	int i;
	for(i = 0 ; i < numOfThreads ; i++){
		pthread_create(new_pool->threads + i, NULL, executer, new_pool);
		new_pool->threadsAlive++;		
	}

	return new_pool;
}


void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {
	printf("[DESTROY] - CALLED\n");
	if (threadPool->has_destroyed==1) {
		return; // this is in case other thread is trying to make destroy (piazza)
	}
	threadPool->has_destroyed = 1;
	threadPool->shouldWaitForTasks = shouldWaitForTasks;

	if (shouldWaitForTasks != 0) {
		pthread_cond_broadcast(&(threadPool->executerCond)); // wake up the threads to work
		while(!osIsQueueEmpty(threadPool->tasks_queue)) {
			printf("[DESTROY] - WAKING THREADS UP\n");
			pthread_cond_broadcast(&(threadPool->executerCond)); // wake up threads so they can wake me up later
			pthread_cond_wait(&threadPool->destroyCond, &threadPool->destroyCondMutex); //go to sleap
			printf("[DESTROY] - WOKE UP\n");
		}
	}

	printf("[DESTROY] - MAKING shouldWaitForTasks TO 0\n");
	threadPool->shouldWaitForTasks = 0; //make sure the threads will not take anymore tasks

	//wait for all running tasks to finish and all threads to die
	while(threadPool->runningTasks!=0 || threadPool->threadsAlive!=0) {
		printf("[DESTROY] - %d RUNNING TASKS %d THREADS ALIVE\n",threadPool->runningTasks,threadPool->threadsAlive );
		
		printf("[DESTROY] - WAKING THREADS UP AND GOING TO SLEEP\n");
		pthread_cond_broadcast(&(threadPool->executerCond)); // wake up threads so they can wake me up later
		pthread_cond_wait(&threadPool->destroyCond, &threadPool->destroyCondMutex);	
		printf("[DESTROY] - WOKE UP\n");	
	}
	
	printf("[DESTROY] - FREEING TASKS FROM QUEUE\n");
	//free tasks
	while(!osIsQueueEmpty(threadPool->tasks_queue)) {
		TaskNode* current = osDequeue(threadPool->tasks_queue);
		free(current);
	}
	//free queue
	printf("[DESTROY] - FREEING QUEUE\n");
	osDestroyQueue(threadPool->tasks_queue);
	int i;
	//join threads
	printf("[DESTROY] - JOIN THREADS\n");
	for(i = 0 ; i < threadPool->numOfThreads ; i++){
		//pthread_cancel(threadPool->threads[i]);
		//pthread_exit(threadPool->threads + i);
		//pthread_cond_broadcast(&(threadPool->executerCond)); //free the destroy in case it was called
		pthread_join(threadPool->threads[i], NULL);	
	}

	//free memory allocated
	printf("[DESTROY] - FREE MEMORY ALLOCATED\n");
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
	TaskNode* newTask = (TaskNode*)malloc(sizeof(TaskNode));
	newTask->param = param;
	newTask->routine = computeFunc;
	osEnqueue(threadPool->tasks_queue, newTask);
	pthread_cond_broadcast(&(threadPool->executerCond)); //free the threads to do some work!
	printf("[INSERT] - FINISH\n");
	return res;	
}





