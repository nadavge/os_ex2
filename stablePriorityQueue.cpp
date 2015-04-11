#include <vector>
#include "uthreads.h"
#include "thread.h"

StablePriorityQueue::StablePriorityQueue()
{

}

void StablePriorityQueue::addThread(Thread *thread);
{
    _threadQueues[thread->priority].insert(thread);
}

void StablePriorityQueue::removeThread(Thread *thread);
{

    vector<Thread*> vec& = _threadQueues[thread->priority];
    // Erase and remove idiom
    vec.erase(std::remove(vec.begin(), vec.end(), thread), vec.end());

}

Thread* StablePriorityQueue::getThreadById(int tid);
{
    for (Priority pri = RED; pri <= GREEN; pri++)
    {

		vector<Thread*> vec& = _threadQueues[pri];
        it = find_if(vec.begin(), vec.end(), [&tid](const Thread* thread)
					{
						return thread.tid == tid;
					});
        if (it != myvector.end())
		{
            return thread;
		}
    }
    return nullptr;

}
Thread* StablePriorityQueue::getTopThread();
{
    for (Priority pri = RED; pri <= GREEN; pri++)
    {

		vector<Thread*> vec& = _threadQueues[pri];
        if (vec.size() > 0)
		{
			return vec.front()
		}
    }
    return nullptr;
}


