#include<concurrent/task.h>
#include<concurrent/actor.h>
#include<concurrent/thread.h>
#include<concurrent/thread_pool.h>
#include<concurrent/ring_buffer_spsc_lockfree.hpp>
#include<concurrent/queue_mpmc.hpp>
#include<concurrent/queue_mpsc.hpp>
#include<concurrent/ring_buffer_mpmc.hpp>

#include <utility/logger.h>

#include <cstddef>
#include <algorithm>
#include <iostream>
#include <vector>
#include <thread>
using namespace std;
using namespace concurrent;

class worker
{
    public :
        void* run(void* arg)
        {
            int* p_val = static_cast<int *>(arg);
            ++(*p_val);           
            return nullptr;
        }
};

class fred : public concurrent::Actor
{
    int foo = 0;
    public:
    
        void *run() override
        {
            foo++;
            cout << "fred" << endl;
            return nullptr;
        }

        int getFoo() const { return foo; }
};

TEST(Concurrent, Thread)
{
    worker w;
    int testVal = 665;

    // Building thread
    TaskPtr task(new Task(&worker::run, &w, static_cast<void *>(&testVal)));
    concurrent::Thread t1;
    t1.setTask(std::move(task));

    t1.start();
    t1.join();

    EXPECT_EQ(666, testVal);
}

TEST(Concurrent, Actor)
{
    fred f;
    f.start();
    f.join();
    int testVal = f.getFoo();
    EXPECT_EQ(1, testVal);
}

TEST(Concurrent, RingBufferSPSCLockFree)
{
    concurrent::RingBufferSPSCLockFree<int> queue(19);
    std::vector<std::thread> threads;
    vector<int> testVector = {4,5,7,2};
    int sum = 0;

    for (auto i : testVector)
    {
        threads.push_back(std::thread([&](){ queue.push(i); }));
        sum += i;
    }

    int testSum = 0;

    for (auto i : testVector)
    {
        threads.push_back(std::thread([&](){ int n = 0; queue.tryPop(&n); cout << n << endl; testSum += n; }));
    }

    for (auto& elem : threads)
    {
        elem.join();
    }

    EXPECT_EQ(sum, testSum);
}


TEST(Concurrent, QueueMPMC)
{
    concurrent::QueueMPMC<int> mqueue;
    std::vector<std::thread> threads;
    vector<int> testVector = { 4, 5, 7, 2 };
    int sum = 0;
    int testSum = 0;

    for (auto i : testVector)
    {
        threads.push_back(std::thread([&](){ mqueue.enqueue(i); }));
        sum += i;
    }

    for (auto i : testVector)
    {
        threads.push_back(std::thread([&](){ int i = 0; mqueue.dequeue(&i); testSum += i; }));
    }

    for (auto& elem : threads)
    {
        elem.join();
    }

    EXPECT_EQ(sum, testSum);
}

TEST(Concurrent, QueueMPSC)
{
    concurrent::QueueMPSC<int> mqueue;
    std::vector<std::thread> threads;
    vector<int> testVector = { 4, 5, 7, 2 };
    int sum = 0;
    int testSum = 0;

    for (auto i : testVector)
    {
        threads.push_back(std::thread([&](){ mqueue.push(i); }));
        sum += i;
    }
  
    std::thread readThread([&]()
                    { 
                        concurrent::QueueMPSC<int>::QueueMPSCNode* node = nullptr;
                        node = mqueue.flush();
                        while (node)
                        {
                            testSum += node->m_data;
                            node = node->m_next;
                        }
                         
                    });

    for (auto& elem : threads)
    {
        elem.join();
    }

    readThread.join();

    EXPECT_EQ(sum, testSum);
}

TEST(Concurrent, RingBufferMPMC)
{
    concurrent::RingBufferMPMC<int> ringBuffer(10);
    std::vector<std::thread> threads;
    vector<int> testVector = { 4, 5, 7, 2};
    int sum = 0;
    int testSum = 0;

    for (auto i : testVector)
    {
        threads.push_back(std::thread([&](){ringBuffer.push(i); }));
        sum += i;
    }

    for (auto i : testVector)
    {
        threads.push_back(std::thread([&](){ int n = ringBuffer.pop(); testSum += n; }));
    }

    for (auto& elem : threads)
    {
        elem.join();
    }

    EXPECT_EQ(sum, testSum);
}

struct ThreadPoolJobArgs
{
    int* array;
    int index;
};

class ThreadPoolJob
{
    public:
        void* run(ThreadPoolJobArgs arg)
        {
            arg.array[arg.index] = 1;
            return nullptr;
        }
};

TEST(Concurrent, ThreadPool)
{
    utility::Logger::getInstance().initialise(1024);
    utility::Logger::getInstance().setLogFile("oms_log.txt");
    utility::Logger::getInstance().enableFileLogging(false);
    utility::Logger::getInstance().enableConsoleOutput(false);

    vector<string> threadNames = { "a", "b", "c", "d" };
    
    concurrent::ThreadPoolArguments args;
    args.m_hyperThreading = true;
    args.m_pinThreadsToCores = true;
    args.m_threadNames = threadNames;
    args.m_workQueueSizePerThread = 1024;
    concurrent::ThreadPool pool;
    
    pool.initialise(args);

    int* workArray = new int[ threadNames.size() ];
    ThreadPoolJob job;
    
    ThreadPoolJobArgs job_args;
    job_args.array = workArray;

    int expectedSum = 0;
    
    for (size_t i = 0; i < threadNames.size(); i++)
    {
        job_args.index = i;
        expectedSum += i;
        concurrent::Task task(&ThreadPoolJob::run, &job, job_args);
        pool.submitTask(task, i);
        concurrent::Thread::sleep(1000);
    }

    pool.shutdown();

    int actualSum = 0;

    std::for_each(workArray, workArray + threadNames.size(), [&] (int val) { actualSum+= val;});

    delete[] workArray;

    EXPECT_EQ(actualSum, 4);
}