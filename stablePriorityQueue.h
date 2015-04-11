#include <vector>
#include "uthreads.h"
#include "thread.h"
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
		* @return true on success, false if not or not found
		**/
		bool removeThread(Thread *thread);
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
		~stablePriorityQueue();
	private:
		vector<Thread*> threadQueues[3];
};
