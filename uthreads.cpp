/*
 * User-Level Threads Library (uthreads)
 * Author: Nadav Geva and Daniel Danon
 */

//================================INCLUDES=============================

#include "uthreads.h"
#include "stablePriorityQueue.h"
#include <sys/time.h>
#include <signal.h>
#include "thread.h"
#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;

//================================DEFINITIONS==========================

#define MAIN_ID 0
#define ERROR -1

#define SYSTEM_ERROR "system error: "
#define TIMER_ERROR "Setting timer failed\n"

#define START_TIMER() setitimer(ITIMER_VIRTUAL, &gTvQuanta, nullptr)
#define STOP_TIMER() setitimer(ITIMER_VIRTUAL, &gTvDisable, nullptr)

enum JumpType
{
	/*
	 * SWITCHING should be first, to act by the spec of sigsetjmp
	 * since sigsetjmp returns 0 on first call
	 */
	SWITCHING,
	RETURN
};

enum SwitchAction
{
	DEF_SWITCH,
	SUSPEND,
	TERMINATE
};

#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
		"rol    $0x11,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
		"rol    $0x9,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#endif

//================================GLOBALS==============================

static Thread *gCurrentThread = nullptr;
static Thread *gThreadToTerminate = nullptr;
static int gTotalQuantums = 0;
static bool threadIdsInUse[MAX_THREAD_NUM] = {false};

static vector <Thread*> blockedThreads;
static StablePriorityQueue priorityQueue;

// A timer interval for a quanta
itimerval gTvQuanta = {0};
// A timer interval for disabling the timer
const itimerval gTvDisable = {0};

//================================DECLARATIONS=========================

Thread* getThreadById(int tid, Location* loc);
int getMinUnusedThreadId();
void switchThreads(SwitchAction action=DEF_SWITCH);
int blockSignals();
int unBlockSignals();
void removeFromBlocked(Thread* thread);

//================================IMPLEMENTATION=======================

/**
* @brief Switch between two user threads, based on RR+ algorithm
*/
void switchThreads(SwitchAction action)
{
	int jumpType = 0;
	Thread* newThread = nullptr;

	if (STOP_TIMER() == ERROR)
	{
		goto error;
	}

	++gTotalQuantums;
	++gCurrentThread->quantums;

	jumpType = sigsetjmp(gCurrentThread->env,1);
	// If returning to run current thread, simply end switch method
	if (jumpType == RETURN)
	{
		// Check if got here from a suiciding thread, if so destroy it
		if (gThreadToTerminate != nullptr)
		{
			delete gThreadToTerminate;
			gThreadToTerminate = nullptr;
		}

		return;
	}

	// TODO implement pop
	newThread = priorityQueue.getTopThread();
	// If there exists a thread to switch to
	if (newThread != nullptr)
	{
		// TODO Maybe implement as queue to be more efficient
		priorityQueue.removeThread(newThread);
		if (action == TERMINATE)
		{
			gThreadToTerminate = gCurrentThread;
		}
		else if (action == DEF_SWITCH)
		{
			priorityQueue.addThread(gCurrentThread);
		}
	}
	else
	{
		// In that case we want to stay the active thread, unless we terminated ourselves
		if (action == TERMINATE)
		{
			// TODO find a way to handle a case of no available replacement on suicide
			// Maybe take control over the main thread with editing pc to be the kill process
		}
	}

	if (START_TIMER() == ERROR)
	{
		goto error;
	}

	siglongjmp(gCurrentThread->env, RETURN);

error:
	cerr << SYSTEM_ERROR TIMER_ERROR;
}

void timerHandler(int sig)
{
	switchThreads();
}

/* Initialize the thread library */
int uthread_init(int quantum_usecs)
{
	blockSignals();
	gTvQuanta.it_value.tv_usec = quantum_usecs;
	// TODO Add to running structure
	gCurrentThread = new Thread(MAIN_ID, ORANGE);
	threadIdsInUse[0] = true;
	signal(SIGVTALRM, timerHandler);
	START_TIMER();
	unBlockSignals();
}

/* Create a new thread whose entry point is f */
int uthread_spawn(void (*f)(void), Priority pr)
{
	address_t sp, pc;
	blockSignals();
	Thread* thread = new Thread(getMinUnusedThreadId(), pr);
	threadIdsInUse[thread->tid] = true;
	sp = (address_t) thread->stack + STACK_SIZE - sizeof(address_t);
	pc = (address_t) f;
	sigsetjmp(thread->env, 1);
	(thread->env->__jmpbuf)[JB_SP] = translate_address(sp);
	(thread->env->__jmpbuf)[JB_PC] = translate_address(pc);
	sigemptyset(&(thread->env)->__saved_mask);
	unBlockSignals();
	return thread->tid;

}

int blockSignals()
{
	sigset_t signal_set;
	sigemptyset(&signal_set);
	sigaddset (&signal_set, SIGINT);
	if (sigprocmask(SIG_BLOCK, &signal_set, NULL) < 0) {
		return -1;
	}

	return 0;
}
int unBlockSignals()
{
	sigset_t signal_set;
	sigemptyset(&signal_set);
	sigaddset (&signal_set, SIGINT);
	if (sigprocmask(SIG_UNBLOCK, &signal_set, NULL) < 0) {
		return -1;
	}

	return 0;
}


/* Terminate a thread */
int uthread_terminate(int tid)
{
	blockSignals();
	if(tid == 0)
	{
		exit(0);
	}
	if(tid < 0)
	{
		unBlockSignals();
		return ERROR;
	}
	Location* loc = nullptr;

	Thread* thread = getThreadById(tid, loc);
	if(thread == nullptr)
	{
		unBlockSignals();
		return ERROR;
	}
	threadIdsInUse[tid] = false;
	switch(*loc)
	{
	case BLOCKED:
		removeFromBlocked(thread);
		break;
	case QUEUE:
		priorityQueue.removeThread(thread);
		break;
	case ACTIVE:
		switchThreads(TERMINATE);
		break;
	}
	unBlockSignals();
	return 0;
}

/* Suspend a thread */
int uthread_suspend(int tid)
{
	blockSignals();
	Location* loc = nullptr;
	Thread* thread = nullptr;
	if(tid <= 0)
	{
		unBlockSignals();
		return ERROR;
	}
	thread = getThreadById(tid, loc);
	if(thread == nullptr)
	{
		unBlockSignals();
		return ERROR;
	}
	switch(*loc)
	{
	case BLOCKED:
		break;
	case QUEUE:
		priorityQueue.removeThread(thread);
		blockedThreads.push_back(thread);
		break;
	case ACTIVE:
		switchThreads(SUSPEND);
		break;

	}
	unBlockSignals();
	return 0;
}

/* Resume a thread */
int uthread_resume(int tid)
{
	blockSignals();
	Location* loc = nullptr;
	Thread* thread = getThreadById(tid, loc);
	if (thread == nullptr)
	{
		unBlockSignals();
		return -1;
	}
	switch(*loc)
	{
	case BLOCKED:
		removeFromBlocked(thread);
		priorityQueue.addThread(thread);
	case QUEUE:
	case ACTIVE:
		break;
	}
	unBlockSignals();
	return 1;
}


/* Get the id of the calling thread */
int uthread_get_tid()
{
	return gCurrentThread->tid;
}


/* Get the total number of library quantums */
int uthread_get_total_quantums()
{
	return gTotalQuantums;
}


/* Get the number of thread quantums */
int uthread_get_quantums(int tid)
{
	blockSignals();
	Location* loc = nullptr;

	Thread* thread = getThreadById(tid, loc);
	if (thread == nullptr)
	{
		unBlockSignals();
		return -1;
	}
	unBlockSignals();
	return thread->quantums;
}

int getMinUnusedThreadId()
{
	int i = 0;
	for(i = 0; i < MAX_THREAD_NUM; i++)
	{
		if(! threadIdsInUse[i])
		{
			unBlockSignals();
			return i;
		}
	}
	return -1;
}

void removeFromBlocked(Thread* thread)
{
	// Erase and remove idiom
    blockedThreads.erase(std::remove(blockedThreads.begin(), blockedThreads.end(), thread), blockedThreads.end());
}

Thread* getThreadById(int tid, Location* loc)
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

	auto it = find_if(blockedThreads.begin(), blockedThreads.end(),
			  [&tid](const Thread* thread)
				{
					return thread->tid == tid;
				});
	if (it != blockedThreads.end())
	{
		if(loc != nullptr)
		{
			*loc = BLOCKED;
		}
		return *it;
	}
	return nullptr;
}
