#ifndef _THREAD_H
#define _THREAD_H

#include <setjmp.h>

/**
* @brief A struct to represent a user-thread
*/
struct Thread {
	// The threads ID
	int tid;
	Priority priority;
	// Quantums received
	int quantums;
	// Holds the environment for the current thread
	sigjmp_buf env;
	// The processes stack
	char stack[STACK_SIZE];
};

#endif //_THREAD_H
