#include "central_order_book.h"
#include <queue>
#include <exception>
#include <boost/format.hpp>
using namespace std;

//#define SINGLE_THREADED

#include <utility/stopwatch.h>
using namespace utility;

namespace order_matcher
{

void CentralOrderBook::accept(Visitor<Order>& v)
{
    for (auto& element : m_orderBookDictionary)
    {
        element.second.accept(v);
    }
}

void CentralOrderBook::initialise(concurrent::ThreadPoolArguments& args)
{
    notify(boost::str(boost::format("Initialising thread pool, work queue size per thread %d")  %args.m_workQueueSizePerThread) );

    int queueID = -1;
    
    // Thread names in args are also symbol names
    for (auto itr : args.m_threadNames)
    {
        auto symbol = itr;
        
        OrderBook currentOrderBook(symbol);
        m_orderBookDictionary.insert( make_pair(symbol, currentOrderBook));

        m_queueIDDictionary.insert( make_pair(symbol, ++queueID) );
    }

    m_orderBookThreadPool.initialise(args);
}
    
bool CentralOrderBook::doesOrderBookExist(const string& symbol) const
{
    if (m_orderBookDictionary.find(symbol) == m_orderBookDictionary.end())
    {
        return false;
    }
    
    return true;
}

bool CentralOrderBook::addOrder(const Order& order)
{
    auto symbol = order.getSymbol();

    if (doesOrderBookExist(symbol) == false)
    {
        // SEND REJECTED MESSAGE TO THE CLIENT
        rejectOrder(order, "Symbol not supported");
        
        return false;
    }

    int queueID = m_queueIDDictionary[symbol];

    // SEND ACCEPTED MESSAGE TO THE CLIENT
    notify(boost::str(boost::format("New order accepted, client %s, client order ID %d ")  %order.getOwner() % order.getClientID() ));
    OutgoingMessage message(order, OutgoingMessageType::ACCEPTED);
    m_outgoingMessages.enqueue(message);

#ifndef SINGLE_THREADED
    // MULTITHREADED MODE : SUBMIT NEW ORDER TASK TO THE THREAD POOL
    concurrent::Task newOrderTask(&CentralOrderBook::taskNewOrder, this, order );
    m_orderBookThreadPool.submitTask(newOrderTask, queueID);
#else
    // SINGLE THREADED MODE : EXECUTE NEW ORDER SYNCHRONOUSLY
    taskNewOrder(order);
#endif
    return true;
}

void* CentralOrderBook::taskNewOrder(const Order& order)
{
    StopWatch watch;
    watch.start();
    ////////////////////////////////////////////////////////////////
    auto symbol = order.getSymbol();
    m_orderBookDictionary[symbol].insert(order);

    queue<Order> processedOrders;
    m_orderBookDictionary[symbol].processMatching(processedOrders);

    auto processedOrderNum = processedOrders.size();

    // Append messages to outgoing queue
    // to let the clients know whether orders filled or partially filled
    while (processedOrders.size())
    {
        auto order = processedOrders.front();
        processedOrders.pop();

        // SEND FILLED OR PARTIALLY FILLED MESSAGE TO THE CLIENT
        OutgoingMessage message(order, order.isFilled() ? OutgoingMessageType::FILLED : OutgoingMessageType::PARTIALLY_FIELD);
        m_outgoingMessages.enqueue(message);
    }

    ////////////////////////////////////////////////////////////////
    watch.stop();
    notify(boost::str(boost::format("Order processing for symbol %s took %07ld milliseconds , num of processed orders : %d") % symbol % watch.getElapsedTimeMilliseconds() % processedOrderNum));
    return nullptr;
}

void CentralOrderBook::rejectOrder(const Order& order, const std::string& message)
{
    notify(boost::str(boost::format("Order being rejected, client %s, client ID %d")  %order.getOwner() %order.getClientID()));
    OutgoingMessage outgoingMessage(order, OutgoingMessageType::REJECTED, message);
    m_outgoingMessages.enqueue(outgoingMessage);
}

void CentralOrderBook::cancelOrder(const Order& order, const std::string& origClientOrderID)
{
    const string& owner = order.getOwner();
    const string& symbol = order.getSymbol();
    // NOTIFY THE CLIENT
    notify(boost::str(boost::format("Order being canceled, client %s, client ID %d") % owner % origClientOrderID));

    int queueID = m_queueIDDictionary[symbol];
    concurrent::Task cancelOrderTask(&CentralOrderBook::taskCancelOrder, this, order, origClientOrderID);

#ifndef SINGLE_THREADED
    // MULTITHREADED MODE :SUBMIT CANCEL TASK TO THE THREADPOOL
    m_orderBookThreadPool.submitTask(cancelOrderTask, queueID);
#else
    // SINGLE THREADED MODE : CANCEL SYNCHRONOUSLY 
    taskCancelOrder(order, origClientOrderID);
#endif
}

void* CentralOrderBook::taskCancelOrder(const Order& order, const std::string& origClientOrderID)
{
    const string& owner = order.getOwner();
    const string& symbol = order.getSymbol();
    const OrderSide& side = order.getSide();
    Order* orderBeingCanceled = nullptr;

    if (m_orderBookDictionary[symbol].find(&orderBeingCanceled, owner, origClientOrderID, side) == true)
    {
        orderBeingCanceled->cancel();
        OutgoingMessage outgoingMessage(*orderBeingCanceled, OutgoingMessageType::CANCELED, "Order canceled");
        m_outgoingMessages.enqueue(outgoingMessage);
        m_orderBookDictionary[symbol].erase(*orderBeingCanceled);
    }
    
    else
    {
        // Reschedule in case the client`s cancel order reached before the actual add request to be canceled
        notify(boost::str(boost::format("Cancel order being rescheduled, client %s, client ID %d") % owner % origClientOrderID));
        cancelOrder(order, origClientOrderID);
    }
        
    return nullptr;
}

}// namespace