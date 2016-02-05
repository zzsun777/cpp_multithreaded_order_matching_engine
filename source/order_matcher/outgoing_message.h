#ifndef __OUTGOING_MESSAGE__
#define __OUTGOING_MESSAGE__

#include <string>
#include "order.h"
#include <memory/aligned.hpp>
#include <utility/pretty_exception.h>

namespace order_matcher
{

enum class OutgoingMessageType { ACCEPTED, FILLED, PARTIALLY_FIELD, CANCELED, REJECTED };

class OutgoingMessage : public memory::Aligned<> 
{
    public:

        OutgoingMessage()
        {
        }
        
        OutgoingMessage(Order order, OutgoingMessageType type, const std::string& message = "") : m_order(order), m_type(type), m_message(message)
        {
        }

        bool hasMessage() const { return m_message.length() != 0 ; }
        const Order& getOrder() const { return m_order; }
        const OutgoingMessageType& getType() const { return m_type; }
		
        std::string toString() const
        {
            switch (m_type)
            {
                case order_matcher::OutgoingMessageType::ACCEPTED:
                    return "ACCEPTED";
                    break;
                case order_matcher::OutgoingMessageType::FILLED:
                    return "FILLED";
                    break;
                case order_matcher::OutgoingMessageType::PARTIALLY_FIELD:
                    return "PARTIALLY_FIELD";
                    break;
                case order_matcher::OutgoingMessageType::CANCELED:
                    return "CANCELED";
                    break;
                case order_matcher::OutgoingMessageType::REJECTED:
                    return "REJECTED";
                    break;
                default:
                    THROW_PRETTY_EXCEPTION(std::string("Invalid outgoing message type"))
                    break;
            }
        }
		
	private:
        Order m_order;
        OutgoingMessageType m_type;
        std::string m_message;
};

} // namespace

#endif