#ifndef _THREAD_H
#define _THREAD_H

#include <setjmp.h>

/**
* @brief A struct to represent a user-thread
*/
struct Thread {
	// The threads ID
	Thread(int id, Priority pri) : tid(id), priority(pri)

	int tid;
	Priority priority;
	// Quantums received
	int quantums;
	// Holds the environment for the current thread
	sigjmp_buf env;
	// The processes stack
	char stack[STACK_SIZE];
};


typedef enum Location {
	QUEUE = 0,
	BLOCKED,
	ACTIVE,
	NOT_FOUND
}
#endif //_THREAD_H
