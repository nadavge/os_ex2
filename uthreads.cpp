/*
 * User-Level Threads Library (uthreads)
 * Author: Nadav Geva and Daniel Danon
 */
#include <uthreads.h>


static int gQuanta = 0;
static Thread *gCurrentThread = NULL;
static int gTotalQuantums = 0;
static bool threadIdsInUse[MAX_THREAD_NUM] = {false};

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
	sigsetjmp(stack->env, 1);
	(stack->env->__jmpbuf)[JB_SP] = translate_address(sp);
	(stack->env->__jmpbuf)[JB_PC] = translate_address(pc);
	sigemptyset(&(stack->env)->__saved_mask);
	return thread->tid;

}

/* Terminate a thread */
int uthread_terminate(int tid)
{

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

