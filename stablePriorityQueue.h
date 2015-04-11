#include <vector>
#include "uthreads.h"
#include "thread.h"
using namespace std;

class StablePriorityQueue 
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