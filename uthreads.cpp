/*
 * User-Level Threads Library (uthreads)
 * Author: Nadav Geva and Daniel Danon
 */
#include <uthreads.h>


static int gQuanta = 0;
static Thread *gCurrentThread = nullptr;
static Thread *gThreadToTerminate = nullptr;
static int gTotalQuantums = 0;
static bool threadIdsInUse[MAX_THREAD_NUM] = {false};

static vector <Thread*> blockedThreads;
static StablePriorityQueue priorityQueue;

/* Initialize the thread library */
int uthread_init(int quantum_usecs)
{

}

/* Create a new thread whose entry point is f */
int uthread_spawn(void (*f)(void), Priority pr)
{
	Thread* thread = new Thread(getMinUnusedThreadId(), pr);
	sp = (address_t) thread->stack + STACK_SIZE - sizeof(address_t);
	pc = (address_t) f;
	sigsetjmp(thread->env, 1);
	(thread->env->__jmpbuf)[JB_SP] = translate_address(sp);
	(thread->env->__jmpbuf)[JB_PC] = translate_address(pc);
	sigemptyset(&(thread->env)->__saved_mask);
	return thread->tid;

}

/* Terminate a thread */
int uthread_terminate(int tid)
{

	threadIdsInUse[tid] = false;
}

/* Suspend a thread */
int uthread_suspend(int tid)
{
	Location* loc;
	#Thread* thread = getThreadById()
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
	return gTotalQuantums;
}


/* Get the number of thread quantums */
int uthread_get_quantums(int tid)
{
	Thread* thread = getThreadById(tid);
	if (thread == nullptr)
	{
		return -1;
	}
	return thread->quantums;
}

int getMinUnusedThreadId()
{
	int i = 0;
	for(i = 0; i < MAX_THREAD_NUM; i++)
	{
		if(! threadIdsInUse[i])
		{
			return i;
		}
	}
	return -1;
}

Thread* getThreadById(int tid, Location* loc = nullptr)
{
	Thread* thread = nullptr;
	if(gCurrentThread->tid == tid)
	{
		if(loc != nullptr)
		{
			*loc = ACTIVE;
		}
		return gCurrentThread;
	}
	thread = priorityQueue.getThreadById(tid);
	if (thread != nullptr)
	{
		if(loc != nullptr)
		{
			*loc = QUEUE;
		}
		return thread;

	}

	it = find_if(blockedThreads.begin(), blockedThreads.end(), [&tid](const Thread* thread)
				{
					return thread.tid == tid;
				});
	if (it != blockedThreads.end())
	{
		if(loc != nullptr)
		{
			*loc = BLOCKED;
		}
		return it;
	}
	return nullptr;
}
