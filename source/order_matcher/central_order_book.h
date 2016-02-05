#ifndef __ORDER_QUEUE__
#define  __ORDER_QUEUE__

#include "order.h"
#include "order_book.h"
#include "outgoing_message.h"

#include <vector>
#include <string>
#include <queue>
#include <unordered_map>

#include <boost/noncopyable.hpp>

#include <concurrent/thread.h>
#include <concurrent/queue_mpmc.hpp>
#include <concurrent/thread_pool.h>

#include <utility/visitor.hpp>
#include <utility/observer.hpp>

namespace order_matcher
{

using OutgoingMessageQueue = concurrent::QueueMPMC<OutgoingMessage>;

class CentralOrderBook : public boost::noncopyable, public utility::Visitable<Order>, public utility::Observable<CentralOrderBook>
{
    public:
        CentralOrderBook() = default;

        ~CentralOrderBook()
        {
            m_orderBookThreadPool.shutdown();
        }

        void accept(utility::Visitor<Order>& v) override;
        void initialise(concurrent::ThreadPoolArguments& args);

        bool addOrder(const Order& order);
        void rejectOrder(const Order& order, const std::string& message);
        void cancelOrder(const Order& order, const std::string& origClientOrderID);

        OutgoingMessageQueue* getOutgoingMessageQueue() { return &m_outgoingMessages; }

    private:
        std::unordered_map<std::string, OrderBook> m_orderBookDictionary; // Symbol - OrderBook dictionary
        std::unordered_map<std::string, int> m_queueIDDictionary; // Symbol - Queue ID dictionary
        bool doesOrderBookExist (const std::string& symbol) const ;

        OutgoingMessageQueue m_outgoingMessages;
        concurrent::ThreadPool m_orderBookThreadPool;

        void* taskNewOrder(const Order& order);
        void* taskCancelOrder(const Order& order, const std::string& origClientOrderID);

        // Move ctor deletion
        CentralOrderBook(CentralOrderBook&& other) = delete;
        // Move assignment operator deletion
        CentralOrderBook& operator=(CentralOrderBook&& other) = delete;
};

} // namespace

#endif