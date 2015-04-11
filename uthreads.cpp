/*
 * User-Level Threads Library (uthreads)
 * Author: Nadav Geva and Daniel Danon
 */
#include <uthreads.h>
#include <sys/time.h>
#include <thread.h>

static int gQuanta = 0;
static Thread *gCurrentThread = nullptr;
static int gTotalQuantums = 0;


// A timer interval for a quanta
itimerval gTvQuanta = {0};
// A timer interval for disabling the timer
itimerval gTvDisable = {0};

/**
* @brief Switch between two user threads, based on RR+ algorithm
*/
void switchThreads()
{
	// TODO Stop timer
	// TODO Save thread state
	// TODO Check if currently returning/switching
	// TODO Switch in case switching (Initiate timer at end)
}

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
