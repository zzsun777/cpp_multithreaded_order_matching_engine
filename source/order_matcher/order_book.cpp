#include "order_book.h"
#include <utility/pretty_exception.h>
#include <utility>
using namespace std;
using namespace utility;

namespace order_matcher
{

OrderBook::OrderBook(const std::string& symbol) : m_symbol(symbol)
{
        
}

void OrderBook::accept(Visitor<Order>& v)
{
    for (auto& element : m_bidOrders)
    {
        v.visit(element.second);
    }

    for (auto& element : m_askOrders)
    {
        v.visit(element.second);
    }
}

void OrderBook::insert(const Order& order)
{
    if (order.getSide() == OrderSide::BUY)
    {
        m_bidOrders.insert( make_pair(order.getPrice(), order));
    }
    else
    {
        m_askOrders.insert( make_pair(order.getPrice(), order));
    }
}

bool OrderBook::find(Order** order, const std::string& owner, const std::string& clientID, OrderSide side)
{
    if (side == OrderSide::BUY)
    {
        for (auto i = m_bidOrders.begin(); i != m_bidOrders.end(); ++i)
        {
            if (i->second.getClientID() == clientID && i->second.getOwner() == owner ) 
            {
                *order = &(i->second);
                return true;
            }
        }
    }
    else if (side == OrderSide::SELL)
    {
        for (auto i = m_askOrders.begin(); i != m_askOrders.end(); ++i)
        {
            if (i->second.getClientID() == clientID && i->second.getOwner() == owner)
            {
                *order = &(i->second);
                return true;
            }
        }
    }

    return false;
}

void OrderBook::erase(const Order& order)
{
    string id = order.getClientID();
    string owner = order.getOwner();

    if (order.getSide() == OrderSide::BUY)
    {
        for (auto i = m_bidOrders.begin(); i != m_bidOrders.end(); ++i)
        { 
            if (i->second.getClientID() == id && i->second.getOwner() == owner)
            {
                m_bidOrders.erase(i);
                return;
            }
        }
    }
    else if (order.getSide() == OrderSide::SELL)
    {
        for (auto i = m_askOrders.begin(); i != m_askOrders.end(); ++i)
        {
            if (i->second.getClientID() == id && i->second.getOwner() == owner)
            {
                m_askOrders.erase(i);
                return;
            }
        }
    }
}

void OrderBook::matchTwoOrders(Order& bid, Order& ask)
{
    double price = ask.getPrice();
    long quantity = 0;

    if (bid.getOpenQuantity() > ask.getOpenQuantity())
    {
        quantity = ask.getOpenQuantity();
    }
    else
    {
        quantity = bid.getOpenQuantity();
    }

    bid.execute(price, quantity);
    ask.execute(price, quantity);
}


// Will return true if any order processed
// Otherwise return false
bool OrderBook::processMatching(queue<Order>& processedOrders)
{
    while (true)
    {
        if (!m_bidOrders.size() || !m_askOrders.size())
        {
            return processedOrders.size() != 0;
        }

        auto iBid = m_bidOrders.begin();
        auto iAsk = m_askOrders.begin();

        if (iBid->second.getPrice() >= iAsk->second.getPrice())
        {
            Order& bid = iBid->second;
            Order& ask = iAsk->second;

            matchTwoOrders(bid, ask);

            processedOrders.push(bid);
            processedOrders.push(ask);

            if (bid.isFilled()) m_bidOrders.erase(iBid);
            if (ask.isFilled()) m_askOrders.erase(iAsk);
        }
        else
        {
            return processedOrders.size() != 0;
        }
    }
}

} // namespace