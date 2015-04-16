#include <vector>
#include "uthreads.h"
#include "thread.h"
#include "stablePriorityQueue.h"
#include <algorithm>

using namespace std;

StablePriorityQueue::StablePriorityQueue()
{

}

StablePriorityQueue::~StablePriorityQueue()
{

}

void StablePriorityQueue::addThread(Thread *thread)
{
    _threadQueues[thread->priority].push_back(thread);
}

void StablePriorityQueue::removeThread(Thread *thread)
{
    vector<Thread*>& vec = _threadQueues[thread->priority];
    // Erase and remove idiom
    vec.erase(std::remove(vec.begin(), vec.end(), thread), vec.end());

}

Thread* StablePriorityQueue::getThreadById(int tid)
{
	auto equalId = [&tid](const Thread* thread) { return thread->tid == tid; };
    for (int pri = RED; pri <= GREEN; ++pri)
    {
		vector<Thread*>& vec = _threadQueues[pri];
        vector<Thread*>::iterator it = find_if(vec.begin(), vec.end(), equalId);
        if (it != vec.end())
		{
            return *it;
		}
    }
    return nullptr;

}
Thread* StablePriorityQueue::getTopThread()
{
    for (int pri = RED; pri <= GREEN; ++pri)
    {
		vector<Thread*>& vec = _threadQueues[pri];
        if (! vec.empty())
		{
			return vec.front();
		}
    }
    return nullptr;
}


Thread* StablePriorityQueue::popBack()
{
	Thread* ret = nullptr;
    for (int pri = GREEN; pri >= RED; --pri)
    {
		vector<Thread*>& vec = _threadQueues[pri];
        if (! vec.empty())
		{
			ret = vec.back();
			vec.pop_back();
			return ret;
		}
    }
    return nullptr;
}
