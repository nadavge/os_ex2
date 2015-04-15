/*
 * User-Level Threads Library (uthreads)
 * Author: Nadav Geva and Daniel Danon
 */

//================================INCLUDES=============================

#include <uthreads.h>
#include <stablePriorityQueue.h>
#include <sys/time.h>
#include <signal.h>
#include <thread.h>
#include <vector>
#include <algorithm>

using namespace std;

//================================DEFINITIONS==========================

#define MAIN_ID 0
#define ERROR -1

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

// TODO remove suspend
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

Thread* getThreadById(int tid, Location* loc=nullptr);
int getMinUnusedThreadId();
void switchThreads(SwitchAction action=DEF_SWITCH);
//TODO implement
void killProcess();

//================================IMPLEMENTATION=======================

/**
* @brief Switch between two user threads, based on RR+ algorithm
*/
void switchThreads(SwitchAction action)
{
	// TODO Handle timer error
	STOP_TIMER();
	int jumpType = sigsetjmp(gCurrentThread->env,1);
	// If returning to run current thread, simply end switch method
	if (jumpType == RETURN)
	{
		// Check if got here from a suiciding thread, if so destroy it
		if (gThreadToTerminate != nullptr)
		{
			delete gThreadToTerminate;
			gThreadToTerminate = nullptr;
		}

		// TODO Handle error timer and change its place
		START_TIMER();
		return;
	}

	Thread* newThread = nullptr;

	// TODO maybe later remove SUSPEND option because just like DEF_SWITCH
	if (true) //(action == DEF_SWITCH || action == SUSPEND)
	{
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
			else
			{
				priorityQueue.addThread(gCurrentThread);
			}
		}
		else
		{
			if (action == TERMINATE)
			{
				// TODO find a way to handle a case of no available replacement on suicide
				// Maybe take control over the main thread with editing pc to be the kill process
			}
		}
	}

	// TODO handle errors
	START_TIMER();
	siglongjmp(gCurrentThread->env, RETURN);
}

void timerHandler(int sig)
{
	switchThreads();
}

/* Initialize the thread library */
int uthread_init(int quantum_usecs)
{
	gTvQuanta.it_value.tv_usec = quantum_usecs;
	// TODO Add to running structure
	gCurrentThread = new Thread(MAIN_ID, ORANGE);
	signal(SIGVTALRM, timerHandler);
	START_TIMER();
}

/* Create a new thread whose entry point is f */
int uthread_spawn(void (*f)(void), Priority pr)
{
	address_t sp, pc;

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
	if(tid == 0)
	{
		exit(0);
	}
	if(tid < 0)
	{
		return ERROR;
	}
	Location* loc = nullptr;

	Thread* thread = getThreadById(tid, loc);
	if(thread == nullptr)
	{
		return ERROR;
	}
	threadIdsInUse[tid] = false;
	switch(*loc)
	{
	case BLOCKED:
		//TODO change to vector remove
		blockedThreads.removeThread(thread);
		break;
	case QUEUE:
		priorityQueue.removeThread(thread);
		break;
	case ACTIVE:
		// TODO handle thread active
		break;

	}
}

/* Suspend a thread */
int uthread_suspend(int tid)
{
	Location* loc = nullptr;
	Thread* thread = nullptr;
	if(tid <= 0)
	{
		return ERROR;
	}
	thread = getThreadById(tid, loc);
	if(thread == nullptr)
	{
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
		// TODO handle thread active
		break;

	}
	return 0;
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
