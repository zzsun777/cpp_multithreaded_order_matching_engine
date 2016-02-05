#ifndef _QUICK_FIX_CONVERTER_
#define _QUICK_FIX_CONVERTER_

#include <exception>

#include "outgoing_message.h"
#include "order.h"

#include <quickfix/Session.h>
#include <quickfix/Values.h>

namespace order_matcher
{

inline OrderType convertOrderTypeFromQuickFix( const FIX::OrdType& ordType )
{
	switch ( ordType )
	{
		case FIX::OrdType_LIMIT: return OrderType::LIMIT;

		default: throw std::runtime_error( "Unsupported Order Type, use limit" );
	}
}

inline OrderSide convertOrderSideFromQuickFix( const FIX::Side& side )
{
	switch ( side )
	{
		case FIX::Side_BUY: return OrderSide::BUY;
 
        case FIX::Side_SELL: return OrderSide::SELL;
		
        default: throw std::runtime_error("Unsupported Side, use buy or sell");
  }
}

inline FIX::Side convertOrderSideToQuickFix(const OrderSide& side)
{
    switch (side)
    {
        case OrderSide::BUY: return FIX::Side_BUY;

        case OrderSide::SELL: return FIX::Side_SELL;

        default: throw std::runtime_error("Unsupported Side, use buy or sell");
    }
}

inline char convertToQuickFixOutgoingMessageType(const OutgoingMessageType& type)
{
    switch (type)
    {
        case OutgoingMessageType::ACCEPTED: return FIX::OrdStatus_NEW;

        case OutgoingMessageType::CANCELED: return FIX::OrdStatus_CANCELED;

        case OutgoingMessageType::FILLED: return FIX::OrdStatus_FILLED;

        case OutgoingMessageType::PARTIALLY_FIELD: return FIX::OrdStatus_PARTIALLY_FILLED;

        case OutgoingMessageType::REJECTED: return FIX::OrdStatus_REJECTED;

        default: throw std::runtime_error("Unsupported outgoing message type");
    }
}

} // namespace

#endif