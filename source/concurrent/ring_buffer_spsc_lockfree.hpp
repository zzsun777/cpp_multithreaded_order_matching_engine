#ifndef _RING_BUFFER_SPSC_LOCKFREE_H_
#define _RING_BUFFER_SPSC_LOCKFREE_H_

#include <cstddef>
#include <atomic>
#include <utility>
#include <memory>
#include <boost/noncopyable.hpp>

namespace concurrent
{

// Bounded queue ( ring buffer ) single producer single consumer queue
// Uses atomic operations with acquire-release memory ordering
template<typename T>
class RingBufferSPSCLockFree : public boost::noncopyable
{
    public:

        explicit RingBufferSPSCLockFree(std::size_t capacity) : m_capacity{capacity}
        {
            assert(capacity > 0);
            m_buffer = new T[m_capacity];
            m_write.store(0);
            m_read.store(0);
        }

        ~RingBufferSPSCLockFree()
        {
            delete [] m_buffer;
        }

        bool tryPush(T val)
        {
            const auto current_tail = m_write.load();
            const auto next_tail = increment(current_tail);
            if (next_tail != m_read.load(std::memory_order_acquire))
            {
                m_buffer[current_tail] = val;
                m_write.store(next_tail, std::memory_order_release);
                return true;
            }

            return false;  
        }

        void push(T val)
        {
            while (!tryPush(val));
        }

        bool tryPop(T* element)
        {
            auto currentHead = m_read.load();
            
            if (currentHead == m_write.load(std::memory_order_acquire))
            {
                return false;
            }

            *element = m_buffer[currentHead];
            m_read.store(increment(currentHead), std::memory_order_release);

            return true;
        }

    private:

        int increment(int n)
        {
            return (n + 1) % m_capacity;
        }

        std::atomic<int> m_write;
        std::atomic<int> m_read;
        std::size_t m_capacity;
        T* m_buffer;

        // Move ctor deletion
        RingBufferSPSCLockFree(RingBufferSPSCLockFree&& other) = delete;
        // Move assignment operator deletion
        RingBufferSPSCLockFree& operator=(RingBufferSPSCLockFree&& other) = delete;
};

}//namespace

#endif