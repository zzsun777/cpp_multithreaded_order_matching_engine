#ifndef _QUEUE_MPSC_
#define _QUEUE_MPSC_

#include <boost/noncopyable.hpp>
#include <mutex>
#include <condition_variable>
#include <memory/aligned_container_policy.h>

namespace concurrent
{

// Unbounded multi producer single consumer queue
template<typename T>
class QueueMPSC : public boost::noncopyable, AlignedContainerPolicy<T>
{
    public:

        struct QueueMPSCNode
        {
            T m_data;
            QueueMPSCNode* m_next;
            QueueMPSCNode() : m_next(nullptr){}
        };

        QueueMPSC()
        {
            // Create a new empty node so we avoid a lock for accessing head in enqueue
            QueueMPSCNode* dummy = new QueueMPSCNode;

            //Tail and head point to it
            m_head = m_tail = dummy;
        }
        
        ~QueueMPSC()
        {
            if (m_head)
            {
                delete m_head;
            }
        }

        void push(T data)
        {
            QueueMPSCNode* newNode = new QueueMPSCNode;
            newNode->m_data = data;
            std::unique_lock<std::mutex> l(m_mutex);
            /////////////////////////////
            m_tail->m_next = newNode;
            m_tail = newNode;
            /////////////////////////////
            l.unlock();
            m_noData.notify_one();
        }

        bool isEmpty()
        {
            bool retVal = true;
            std::lock_guard<std::mutex> l(m_mutex);
            if ( m_head->m_next != nullptr )
            {
                retVal = false;
            }
            return retVal;
        }

        QueueMPSCNode* flush()
        {
            QueueMPSCNode* ret = nullptr;

            if (isEmpty() == true)
            {
                return ret;
            }

            std::unique_lock<std::mutex> l(m_mutex);
            m_noData.wait(l, [this](){return m_head->m_next != nullptr; });
            /////////////////////////////
			//JUST SWAP THE POINTERS
            ret = m_head->m_next;
            m_head->m_next = nullptr;
            m_tail = m_head;
            /////////////////////////////
            l.unlock();

            return ret;
        }

    private:

        QueueMPSCNode* m_head;
        QueueMPSCNode* m_tail;

        std::mutex m_mutex;
        std::condition_variable m_noData;

        // Move ctor deletion
        QueueMPSC(QueueMPSC&& other) = delete;
        // Move assignment operator deletion
        QueueMPSC& operator=(QueueMPSC&& other) = delete;
};

}//namespace

#endif