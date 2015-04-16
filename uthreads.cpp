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
#include "utils.h"

using namespace std;

//================================GLOBALS==============================

static Thread *gCurrentThread = nullptr;
static Thread *gThreadToTerminate = nullptr;
static int gTotalQuantums = 0;
static bool threadIdsInUse[MAX_THREAD_NUM] = {false};

static vector <Thread*> blockedThreads;
static StablePriorityQueue priorityQueue;

// A timer interval for a quanta
itimerval gTvQuanta = {{0}};
// A timer interval for disabling the timer
const itimerval gTvDisable = {{0}};
sigset_t signal_set = {{0}};

//================================DECLARATIONS=========================

Thread* getThreadById(int tid, Location& loc);
int getMinUnusedThreadId();
void switchThreads(SwitchAction action=DEF_SWITCH);
inline void blockSignals();
inline void unBlockSignals();
void removeFromBlocked(Thread* thread);
void releaseThreads();

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

	jumpType = sigsetjmp(gCurrentThread->env, 1);
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

	newThread = priorityQueue.getTopThread();
	// If there exists a thread to switch to
	if (newThread != nullptr)
	{
		priorityQueue.removeThread(newThread);
		switch (action)
		{
		case TERMINATE:
			gThreadToTerminate = gCurrentThread;
			break;
		case DEF_SWITCH:
			priorityQueue.addThread(gCurrentThread);
			break;
		case SUSPEND:
			blockedThreads.push_back(gCurrentThread);
			break;
		}

		gCurrentThread = newThread;
	}

	if (START_TIMER() == ERROR)
	{
		goto error;
	}
	siglongjmp(gCurrentThread->env, RETURN);

error:
	HANDLE_SYSTEM_ERROR(TIMER_ERROR);
}

/**
* @brief A callback function from a clock interrupt
*
* @param sig clock signal
*/
void timerHandler(int sig)
{
	switchThreads();
}

/* Initialize the thread library */
int uthread_init(int quantum_usecs)
{
	// Check if init was already called
	if (threadIdsInUse[MAIN_ID])
	{
		HANDLE_LIBRARY_ERROR(INVALID_CALL_ERROR);
		return ERROR;
	}

	// Set the signal blocking object
	sigemptyset(&signal_set);
	sigaddset (&signal_set, SIGINT);
	
	if (quantum_usecs <= 0)
	{
		HANDLE_LIBRARY_ERROR(TIMER_ERROR);
		return ERROR;
	}

	gTvQuanta.it_value.tv_sec = quantum_usecs/MICRO;
	gTvQuanta.it_value.tv_usec = quantum_usecs%MICRO;
	gCurrentThread = new Thread(MAIN_ID, ORANGE);
	gTotalQuantums = 1;
	threadIdsInUse[MAIN_ID] = true;
	signal(SIGVTALRM, timerHandler);
	if (START_TIMER() == ERROR)
	{
		HANDLE_SYSTEM_ERROR(TIMER_ERROR);
	}

	return 0;	
}

/* Create a new thread whose entry point is f */
int uthread_spawn(void (*f)(void), Priority pr)
{
	address_t sp, pc;
	int threadId = ERROR;
	blockSignals();
	threadId = getMinUnusedThreadId();
	if (threadId == ERROR)
	{
		HANDLE_LIBRARY_ERROR(TOO_MANY_THREADS_ERROR);
		return ERROR;
	}
	Thread* thread = new Thread(threadId, pr);
	threadIdsInUse[thread->tid] = true;
	sp = (address_t) thread->stack + STACK_SIZE - sizeof(address_t);
	pc = (address_t) f;
	sigsetjmp(thread->env, 1);
	(thread->env->__jmpbuf)[JB_SP] = translate_address(sp);
	(thread->env->__jmpbuf)[JB_PC] = translate_address(pc);
	sigemptyset(&(thread->env)->__saved_mask);
	priorityQueue.addThread(thread);
	unBlockSignals();
	return thread->tid;

}

/**
* @brief Block the incoming processor signals
*/
inline void blockSignals()
{
	if (sigprocmask(SIG_BLOCK, &signal_set, NULL) < 0) {
		HANDLE_SYSTEM_ERROR(SIGNAL_ERROR);
	}
}

/**
* @brief Unblock the incoming process signals
*/
inline void unBlockSignals()
{
	if (sigprocmask(SIG_UNBLOCK, &signal_set, NULL) < 0) {
		HANDLE_SYSTEM_ERROR(SIGNAL_ERROR);
	}
}


/* Terminate a thread */
int uthread_terminate(int tid)
{
	Thread* thread = nullptr;
	blockSignals();
	if(tid == MAIN_ID)
	{
		releaseThreads();
		exit(0);
	}
	if(tid < 0)
	{
		HANDLE_LIBRARY_ERROR(ACCESS_NULL_THREAD_ERROR);
		unBlockSignals();
		return ERROR;
	}
	Location loc = NOT_FOUND;

	thread = getThreadById(tid, loc);
	if(thread == nullptr)
	{
		HANDLE_LIBRARY_ERROR(ACCESS_NULL_THREAD_ERROR);
		unBlockSignals();
		return ERROR;
	}
	threadIdsInUse[tid] = false;
	switch(loc)
	{
	case BLOCKED:
		removeFromBlocked(thread);
		delete thread;
		break;
	case QUEUE:
		priorityQueue.removeThread(thread);
		delete thread;
		break;
	case ACTIVE:
		if (gThreadToTerminate != nullptr)
		{
			delete gThreadToTerminate;
			gThreadToTerminate = nullptr;
		}
		switchThreads(TERMINATE);
		break;
	// Put in order to avoid compilation warnings
	default:
		break;
	}
	unBlockSignals();
	return 0;
}

/* Suspend a thread */
int uthread_suspend(int tid)
{
	blockSignals();
	Location loc = NOT_FOUND;
	Thread* thread = nullptr;
	if(tid <= 0)
	{
		HANDLE_LIBRARY_ERROR(ACCESS_NULL_THREAD_ERROR);
		unBlockSignals();
		return ERROR;
	}
	thread = getThreadById(tid, loc);
	if(thread == nullptr)
	{
		HANDLE_LIBRARY_ERROR(ACCESS_NULL_THREAD_ERROR);
		unBlockSignals();
		return ERROR;
	}
	switch(loc)
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
	// Put to avoid compilation warning
	default:
		break;
	}
	unBlockSignals();
	return 0;
}

/* Resume a thread */
int uthread_resume(int tid)
{
	blockSignals();
	Location loc = NOT_FOUND;
	Thread* thread = getThreadById(tid, loc);
	if (thread == nullptr)
	{
		HANDLE_LIBRARY_ERROR(ACCESS_NULL_THREAD_ERROR);
	}
	switch(loc)
	{
	case BLOCKED:
		removeFromBlocked(thread);
		priorityQueue.addThread(thread);
	default:
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
	int threadQuantums = 0;
	blockSignals();
	Location loc = NOT_FOUND;
	Thread* thread = getThreadById(tid, loc);
	if (thread == nullptr)
	{
		HANDLE_LIBRARY_ERROR(ACCESS_NULL_THREAD_ERROR);
		unBlockSignals();
		return -1;
	}
	threadQuantums = thread->quantums;
	if (thread == gCurrentThread)
	{
		// Technically since this quanta has already started it still counts
		++threadQuantums;
	}
	
	unBlockSignals();
	return threadQuantums;
}

/**
* @brief Get the minimum unused thread id available
*
* @return ^^
*/
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

	return ERROR;
}

/**
* @brief Remove thread from the blocked state
*
* @param thread the thread
*/
void removeFromBlocked(Thread* thread)
{
	// Erase and remove idiom
    blockedThreads.erase(std::remove(blockedThreads.begin(), blockedThreads.end(), thread), blockedThreads.end());
}

/**
* @brief Get the thread by its ID
*
* @param tid the ID
* @param loc the thread location in the state graph (by reference)
*
* @return the thread
*/
Thread* getThreadById(int tid, Location& loc)
{
	Thread* thread = nullptr;
	if(tid < 0)
	{
		return nullptr;
	}
	if(gCurrentThread->tid == tid)
	{
		loc = ACTIVE;
		return gCurrentThread;
	}
	thread = priorityQueue.getThreadById(tid);
	if (thread != nullptr)
	{
		loc = QUEUE;
		return thread;

	}

	auto it = find_if(blockedThreads.begin(), blockedThreads.end(),
	          [&tid](const Thread* thread)
				{
					return thread->tid == tid;
				});
	if (it != blockedThreads.end())
	{
		loc = BLOCKED;
		return *it;
	}
	return nullptr;
}

void releaseThreads()
{
	Thread* thread;
	while ( (thread = priorityQueue.popBack()) != nullptr)
	{
		delete thread;
	}

	for (auto thread : blockedThreads)
	{
		delete thread;
	}
}
