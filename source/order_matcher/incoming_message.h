#ifndef _INCOMING_MESSAGE_H_
#define _INCOMING_MESSAGE_H_

#include <exception>
#include <string>
#include "order.h"
#include <memory/aligned.hpp>
#include <utility/pretty_exception.h>

namespace order_matcher
{

enum class IncomingMessageType { NEW_ORDER, CANCEL_ORDER };

class IncomingMessage : public memory::Aligned<> 
{
    public:

        IncomingMessage()
        {
        }
        
        IncomingMessage(Order order, IncomingMessageType type, const std::string& origClientOrderID="") 
        : m_order(order), m_originalOrderID(origClientOrderID), m_type(type)
        {
        }

        const Order& getOrder() const { return m_order; }
        const IncomingMessageType& getType() const { return m_type; }
        const std::string& getOrigClientOrderID() const { return m_originalOrderID; }
        
        std::string toString() const throw(std::invalid_argument)
        {
            switch (m_type)
            {
                case order_matcher::IncomingMessageType::NEW_ORDER:
                    return "NEW_ORDER";
                    break;
                case order_matcher::IncomingMessageType::CANCEL_ORDER:
                    return "CANCEL_ORDER";
                    break;
                default:
                    THROW_PRETTY_INVALID_ARG_EXCEPTION(std::string("Invalid incoming message type"))
                    break;
            }
        }
        
    private:
        Order m_order;
        std::string m_originalOrderID; // Only applies to cancel messages
        IncomingMessageType m_type;
};

} // namespace

#endif