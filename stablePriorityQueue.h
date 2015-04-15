#ifndef _STABLEPRIORITYQUEUE_H
#define _STABLEPRIORITYQUEUE_H

#include <vector>
#include "thread.h"
#include "uthreads.h"

using namespace std;

class StablePriorityQueue
{
	public:
		/**
		* @brief constructor - initializes the queue
		**/
        StablePriorityQueue();
		/**
		* @brief adds a thread to the queue
		* @param thread - pointer to the thread
		**/
		void addThread(Thread *thread);
		/**
		* @brief removes a thread by pointer from the queue
		* @param thread - pointer to the thread
		**/
		void removeThread(Thread *thread);
		/**
		* @brief gets a thread by its id
		* @param id - thread's id
		* @return pointer to the thread
		**/
		Thread* getThreadById(int tid);
		/**
		* @brief gets the thread with the highest priority
		* @return pointer to the thread
		**/
		Thread* getTopThread();


		/**
		* @brief destructor - frees everything that's needed
		**/
		~StablePriorityQueue();
	private:
		vector<Thread*> _threadQueues[GREEN + 1];
};

#endif //_STABLEPRIORITYQUEUE_H
