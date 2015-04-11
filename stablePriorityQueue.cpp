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

}

Thread* StablePriorityQueue::getTopThread();
{

}


