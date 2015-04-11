/*
 * User-Level Threads Library (uthreads)
 * Author: Nadav Geva and Daniel Danon
 */
#include <uthreads.h>


static int gQuanta = 0;
static Thread *gCurrentThread = nullptr;
static int gTotalQuantums = 0;
static bool threadIdsInUse[MAX_THREAD_NUM] = {false};
vector <Thread*> blockedThreads;
StablePriorityQueue priorityQueue;

/* Initialize the thread library */
int uthread_init(int quantum_usecs)
{

}

/* Create a new thread whose entry point is f */
int uthread_spawn(void (*f)(void), Priority pr)
{

}

/* Terminate a thread */
int uthread_terminate(int tid)
{

	threadIdsInUse[tid] = false;
}

/* Suspend a thread */
int uthread_suspend(int tid)
{

}

/* Resume a thread */
int uthread_resume(int tid)
{

}


/* Get the id of the calling thread */
int uthread_get_tid()
{

}


/* Get the total number of library quantums */
int uthread_get_total_quantums()
{

}


/* Get the number of thread quantums */
int uthread_get_quantums(int tid)
{

}

Thread* getThreadById(int tid)
{
	Thread* thread = nullptr;
	if(gCurrentThread->tid == tid)
	{
		return gCurrentThread;
	}
	thread = priorityQueue.getThreadById(tid);
	if (thread != nullptr)
	{
		return thread;

	}

	it = find_if(blockedThreads.begin(), blockedThreads.end(), [&tid](const Thread* thread)
				{
					return thread.tid == tid;
				});
	if (it != blockedThreads.end())
	{
		return it;
	}
	return nullptr;
}
