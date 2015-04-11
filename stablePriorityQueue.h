#include <vector>
#include "uthreads.h"
#include "thread.h"
using namespace std;

class stablePriorityQueue 
{
	public:
		stablePriorityQueue();
		void addThread(Thread *thread);
		bool removeThread(Thread *thread);
		Thread* getThreadById(int id);
		Thread* getTopThread();
		
		~stablePriorityQueue();
	private:
		vector<Thread*> threadQueues[3];
};