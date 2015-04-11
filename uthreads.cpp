/*
 * User-Level Threads Library (uthreads)
 * Author: Nadav Geva and Daniel Danon
 */
#include <uthreads.h>



struct Thread {
	int tid;
	Priority priority;
	address_t entry;
	int quantums;
	char stack[STACK_SIZE];
};

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

