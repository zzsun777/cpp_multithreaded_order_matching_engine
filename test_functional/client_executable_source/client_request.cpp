#include <exception>
#include <iterator>
#include <utility/pretty_exception.h>
#include "client_request.h"
using namespace std;

#include <boost/format.hpp>
#include <boost/tokenizer.hpp>

ClientRequest::ClientRequest(const string& csvRequest)
{
    m_requestSent.store(false);

    boost::char_separator<char> seperator(",");
    boost::tokenizer<boost::char_separator<char>> tokenizer(csvRequest, seperator);
    
    auto iter = tokenizer.begin();
    auto numTokens = std::distance(tokenizer.begin(), tokenizer.end());

    if (numTokens < 5)
    {
        auto exceptionMessage = boost::str(boost::format("Invalid number of tokens in line : %s") % csvRequest);
        THROW_PRETTY_EXCEPTION(exceptionMessage)
    }

    // ORDER TYPE COLUMN
    string orderType = *iter;

    if (orderType.compare("NEW_ORDER") == 0)
    {
        m_type = ClientRequestType::NEW_ORDER;
        
        if (numTokens != 6)
        {
            auto exceptionMessage = boost::str(boost::format("Invalid number of tokens for a new order in line : %s") % csvRequest);
            THROW_PRETTY_EXCEPTION(exceptionMessage)
        }
    }
    else if (orderType.compare("CANCEL_ORDER") == 0)
    {
        m_type = ClientRequestType::CANCEL_ORDER;

        if (numTokens != 5)
        {
            auto exceptionMessage = boost::str(boost::format("Invalid number of tokens for a cancel order in line : %s") % csvRequest);
            THROW_PRETTY_EXCEPTION(exceptionMessage)
        }
    }
    else
    {
        THROW_PRETTY_EXCEPTION(std::string("Invalid order type"))
    }
        
    // SYMBOL COLUMN
    iter++;
    string temp( *iter);
    m_symbol = temp;

    // SIDE COLUMN
    iter++;
    string orderSide = *iter;

    if (orderSide.compare("BUY") == 0)
    {
        m_side = FIX::Side_BUY;
    }
    else if (orderSide.compare("SELL") == 0)
    {
        m_side = FIX::Side_SELL;
    }
    else
    {
        THROW_PRETTY_EXCEPTION(std::string("Invalid side"))
    }

    // TARGET ID
    iter++;
    m_targetID = *iter;

    // ORIG ORDER ID , IF CANCEL ORDER
    if (m_type == ClientRequestType::CANCEL_ORDER)
    {
        iter++;
        m_origOrderID = *iter;
    }
    else // PRICE AND QUANTITY , IF NEW ORDER
    {
        iter++;
        m_price = std::stod(*iter);

        iter++;
        m_quantity = std::stol(*iter);
    }
}

ClientRequest::ClientRequest(const ClientRequest& other)
{
    this->m_requestSent.store( other.requestSent() );
    this->m_orderID = other.getOrderID();
    this->m_origOrderID = other.getOrigOrderID();
    this->m_side = other.getSide();
    this->m_type = other.getType();
    this->m_price = other.getPrice();
    this->m_quantity = other.getQuantity();
    this->m_senderID = other.getSenderID();
    this->m_targetID = other.getTargetID();
}