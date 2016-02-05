#ifndef _ORDER_BOOK_H_
#define _ORDER_BOOK_H_

#include "order.h"

#include <map>
#include <queue>
#include <string>
#include <functional>
#include <memory>

#include <utility/visitor.hpp>

namespace order_matcher
{

class OrderBook : public utility::Visitable<Order>
{
    public :
        OrderBook() = default;
        explicit OrderBook(const std::string& symbol);

        void accept(utility::Visitor<Order>& v) override;

        void insert(const Order& order);
        bool find(Order** order, const std::string& owner, const std::string& clientID, OrderSide side);
        void erase(const Order& order);
        bool processMatching( std::queue<Order>& processedOrders );
        bool isEmpty() const { return (m_bidOrders.size() == 0) && (m_askOrders.size() == 0); }

    private:
        std::string m_symbol;
        
        // Bid orders , place the greatest on top
        std::multimap < double, Order, std::greater < double > > m_bidOrders;
        // Ask orders , place the lowest on top
        std::multimap < double, Order, std::less < double > > m_askOrders;

        void matchTwoOrders(Order& bid, Order& ask);
};

using OrderBookPtr = std::unique_ptr<order_matcher::OrderBook>;

} //namespace

#endif