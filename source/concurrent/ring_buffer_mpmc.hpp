#ifndef _RING_BUFFER_MPMC_H_
#define _RING_BUFFER_MPMC_H_

#include <cstddef>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <utility>
#include <boost/noncopyable.hpp>

namespace concurrent
{

// Bounded queue ( ring buffer ) multi producer multi consumer
// Uses mutex and cond variable
template<typename T>
class RingBufferMPMC : public boost::noncopyable
{
    public:

        explicit RingBufferMPMC(std::size_t capacity) : m_capacity{capacity}, m_front{0}, m_rear{0}, m_count{0}
        {
            assert(capacity > 0);
            std::unique_ptr <T, BufferDeleter> buffer(new T[m_capacity]);
            m_buffer = std::move(buffer);
        }

        ~RingBufferMPMC()
        {
        }

        std::size_t count()
        {
            std::unique_lock<std::mutex> l(m_lock);
            return m_count;
        }

        void push(T data)
        {
            std::unique_lock<std::mutex> l(m_lock);

            m_notFull.wait(l, [this](){return m_count != m_capacity; });

            m_buffer.get()[m_rear] = data;
            m_rear = (m_rear + 1) % m_capacity;
            ++m_count;

            l.unlock();
            m_notEmpty.notify_one();
        }

        T pop()
        {
            std::unique_lock<std::mutex> l(m_lock);

            m_notEmpty.wait(l, [this](){return m_count != 0; });

            T result = m_buffer.get()[m_front];
            m_front = (m_front + 1) % m_capacity;
            --m_count;

            l.unlock();

            m_notFull.notify_one();

            return result;
        }

    private:
        size_t m_capacity;

        int m_front;
        int m_rear;
        std::size_t m_count;

        std::mutex m_lock;
        std::condition_variable m_notFull;
        std::condition_variable m_notEmpty;

        struct BufferDeleter 
        {
            void operator()(T* memory) { delete[] memory; }
        };

        std::unique_ptr<T, BufferDeleter> m_buffer;
};
  

}//namespace

#endif