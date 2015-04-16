#ifndef _THREAD_H
#define _THREAD_H

#include <setjmp.h>

/**
* @brief A struct to represent a user-thread
*/
struct Thread {
	// Constructor for thread
	// @TODO Possibly change quantums to zero and check at the get_quantums function for 0
	Thread(int id, Priority pri) : tid(id), priority(pri), quantums(1) {}

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

enum Location {
	QUEUE = 0,
	BLOCKED,
	ACTIVE,
	NOT_FOUND
};

#endif //_THREAD_H
