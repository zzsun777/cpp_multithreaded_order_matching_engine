#ifndef _QUICK_FIX_CONVERTER_H_
#define _QUICK_FIX_CONVERTER_H_

#include <exception>

#include "outgoing_message.h"
#include "order.h"

#include <quickfix/Session.h>
#include <quickfix/Values.h>

namespace order_matcher
{

inline OrderType convertOrderTypeFromQuickFix(const FIX::OrdType& ordType) throw(std::invalid_argument)
{
	switch ( ordType )
	{
		case FIX::OrdType_LIMIT: return OrderType::LIMIT;

        default: throw std::invalid_argument("Unsupported Order Type, use limit");
	}
}

inline OrderSide convertOrderSideFromQuickFix(const FIX::Side& side) throw(std::invalid_argument)
{
	switch ( side )
	{
		case FIX::Side_BUY: return OrderSide::BUY;
 
        case FIX::Side_SELL: return OrderSide::SELL;
		
        default: throw std::invalid_argument("Unsupported Side, use buy or sell");
  }
}

inline FIX::Side convertOrderSideToQuickFix(const OrderSide& side) throw(std::invalid_argument)
{
    switch (side)
    {
        case OrderSide::BUY: return FIX::Side_BUY;

        case OrderSide::SELL: return FIX::Side_SELL;

        default: throw std::invalid_argument("Unsupported Side, use buy or sell");
    }
}

inline char convertToQuickFixOutgoingMessageType(const OutgoingMessageType& type) throw(std::invalid_argument)
{
    switch (type)
    {
        case OutgoingMessageType::ACCEPTED: return FIX::OrdStatus_NEW;

        case OutgoingMessageType::CANCELED: return FIX::OrdStatus_CANCELED;

        case OutgoingMessageType::FILLED: return FIX::OrdStatus_FILLED;

        case OutgoingMessageType::PARTIALLY_FIELD: return FIX::OrdStatus_PARTIALLY_FILLED;

        case OutgoingMessageType::REJECTED: return FIX::OrdStatus_REJECTED;

        default: throw std::invalid_argument("Unsupported outgoing message type");
    }
}

} // namespace

#endif