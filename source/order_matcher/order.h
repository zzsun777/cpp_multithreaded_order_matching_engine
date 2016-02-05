#ifndef _ORDER_
#define _ORDER_

#include <string>
#include <boost/flyweight.hpp>

namespace order_matcher
{

enum class OrderSide { BUY, SELL };
enum class OrderType { LIMIT,  };

using FlyweightStdString = boost::flyweights::flyweight<std::string>;

class Order
{    
    public:
        
        Order(){}
        Order(std::string clientOrderID, std::string symbol, std::string owner, std::string target, OrderSide side, OrderType type, double price, long quantity);
        void execute(double price, long quantity);
        void cancel() { m_cancelled = true; }
        std::string toString() const;

        bool isFilled() const { return m_executedQuantity == m_quantity; }
        bool isCancelled() const { return m_cancelled; }

        long getQuantity() const { return m_quantity; }
        long getOpenQuantity() const { return m_openQuantity; }
        long getExecutedQuantity() const { return m_executedQuantity; }
        double getPrice() const { return m_price; }
        const std::string& getClientID() const { return m_clientOrderID; }
        const std::string& getSymbol() const { return m_symbol; }
        const std::string& getOwner() const { return m_owner; }
        const std::string& getTarget() const { return m_target; }
        OrderSide getSide() const { return m_side; }
        OrderType getOrderType() const { return m_orderType; } 
        double getAverageExecutedPrice() const { return m_averageExecutedPrice; }
        double getLastExecutedPrice() const { return m_lastExecutedPrice; }
        long getLastExecutedQuantity() const { return m_lastExecutedQuantity; }

        friend std::ostream& operator<<(std::ostream& os, Order& entry);

    private:

        std::string m_clientOrderID;
        FlyweightStdString m_symbol;
        std::string m_owner;
        FlyweightStdString m_target;
        OrderSide m_side;
        OrderType m_orderType;
        double m_price;
        long m_quantity;

        long m_openQuantity;
        long m_executedQuantity;
        bool m_cancelled;
        double m_averageExecutedPrice;
        double m_lastExecutedPrice;
        long m_lastExecutedQuantity;
};

} // namespace

#endif