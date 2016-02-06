#include <cassert>
#include <utility>
using namespace std;

#include <boost/format.hpp>
#include "thread_pool.h"
#include <utility/logger.h>

namespace concurrent
{

void ThreadPool::initialise(const ThreadPoolArguments& args)
{
    assert(args.m_threadNames.size()> 0);

    auto numCores = Thread::getNumberOfCores();

    bool useProcessorsWithEvenIndex = false;

    // If the args says we shall not use it and also in the system if it is off
    if (Thread::isHyperThreading() && args.m_hyperThreading == false)
    {
        // This one is on if we want to avoid hyperthreading
        // Using cores with even indexes will help us to use only physical cores
        // whereas odd indexes are logical cores
        useProcessorsWithEvenIndex = true;
        numCores /= 2;
    }
    
    auto numThreads = args.m_threadNames.size();
    size_t threadID{0};
    string threadName;

    m_threadArguments.reserve(numThreads);
    m_threads.reserve(numThreads);
    m_threadQueues.reserve(numThreads);
    
    for (size_t i{0}; i< numThreads; i++)
    {
        ThreadPoolQueuePtr queue(new ThreadPoolQueue(args.m_workQueueSizePerThread));
        m_threadQueues.push_back(std::move(queue));

        ThreadArgument argument(this, threadID);
        m_threadArguments.emplace_back(argument);

        TaskPtr task(new Task(&ThreadPool::workerThreadFunction, static_cast<void *>(&m_threadArguments.back())));

        threadName = args.m_threadNames[m_threads.size()];
        ThreadPtr thread(new Thread(threadName));
        m_threads.push_back(std::move(thread));

        m_threads.back()->setTask(std::move(task));
        m_threads.back()->start( args.m_threadStackSize );

        //If we need to pin threads to specific cores
        if ( args.m_pinThreadsToCores == true )
        {
            int coreID = i%numCores;

            if (useProcessorsWithEvenIndex)
            {
                // If we want to avoid HT
                // that means we previously halved down num cores so that it is only physical cores
                // Also still we need to use even indexes , as odd indexes are logical cores
                coreID *= 2;
            }

            m_threads.back()->bindThreadToCPUCore(coreID);
        }

        threadID++;
    }
}

// THIS METHOD ITSELF IS NOT THREAD SAFE
void ThreadPool::submitTask(const Task& task, size_t queueID) throw(std::invalid_argument)
{
    if( queueID >= m_threadQueues.size() )
    {
        throw std::invalid_argument("Thread pool submit task , queue index is invalid");
    }
    
    m_threadQueues[queueID]->push(task);
}

void ThreadPool::shutdown()
{
    m_isShuttingDown.store(true);
    for (auto& thread : m_threads)
    {
        thread->join();
    }
}

void* ThreadPool::workerThreadFunction(void* argument)
{
    ThreadArgument* threadArgument = static_cast<ThreadArgument*>(argument);
    ThreadPool* pool = threadArgument->m_threadPool;
    auto queueIndex = threadArgument->m_queueIndex;

    LOG_INFO("Thread pool", boost::str(boost::format("Thread(%d) %s starting") % queueIndex % pool->m_threads[queueIndex]->getThreadName()))
    
    while( ! pool->m_isShuttingDown.load() )
    {
        Task queueTask;
        if ( pool->m_threadQueues[queueIndex]->tryPop(&queueTask) )
        {
            LOG_INFO("Thread pool", pool->m_threads[queueIndex]->getThreadName() + " thread got a new task to execute")
            queueTask.execute();
        }
        else
        {
            concurrent::Thread::yield();
        }
    }

    LOG_INFO("Thread pool", boost::str(boost::format("Thread(%d) %s exiting") % queueIndex % pool->m_threads[queueIndex]->getThreadName()))
    return nullptr;
}
    
}//namespace