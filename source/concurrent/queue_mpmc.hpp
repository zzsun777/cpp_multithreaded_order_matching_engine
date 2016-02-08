#ifndef _QUEUE_MPMC_H_
#define _QUEUE_MPMC_H_

#include <boost/noncopyable.hpp>
#include <mutex>
#include <memory/aligned_container_policy.hpp>

namespace concurrent
{

// Unbounded , multiproducer multiconsumer queue implementation
// Uses fine-grained locking to minimise lock contention :
// 1 mutex for the tail , 1 mutex for the head
template< typename T>
class QueueMPMC : public boost::noncopyable, AlignedContainerPolicy<T>
{
    public:
        
        QueueMPMC()
        {
            // Create a new empty node so we avoid a lock for accessing head in enqueue
            Node* dummy = new Node;

            //Tail and head point to it
            m_head = m_tail = dummy;
        }

        ~QueueMPMC()
        {            
            while (m_head)
            {
                auto temp = m_head;
                m_head = m_head->m_next;
                delete temp;
            }
        }
        
        void enqueue(T data)
        {
            Node* newNode =  new Node;
            newNode->m_data = data;
            
            std::lock_guard<std::mutex> tailLock(m_tailMutex);
            m_tail->m_next = newNode;
            m_tail = newNode;
         }
        
        bool dequeue(T* data)
        {
            m_headMutex.lock();
            
            Node* currentHead = m_head;
            Node* newHead = currentHead->m_next;
            
            if( newHead == nullptr)
            {
                m_headMutex.unlock();
                return false;
            }
            
            *data = newHead->m_data;
            m_head = newHead; // Swapping dummy-initial node so we avoid to update the tail pointer
                             // Therefore no need for protecting the tail
            
            m_headMutex.unlock();
            
            delete(currentHead); // De allocate previous dummy node
            return true;
        }

    private:

        struct Node
        {
            T m_data;
            Node* m_next;
            Node() : m_next(nullptr){}
        };

        std::mutex m_headMutex;
        std::mutex m_tailMutex;

        Node* m_head;
        Node* m_tail;

        // Move ctor deletion
        QueueMPMC(QueueMPMC&& other) = delete;
        // Move assignment operator deletion
        QueueMPMC& operator=(QueueMPMC&& other) = delete;
};

}// namespace
#endif