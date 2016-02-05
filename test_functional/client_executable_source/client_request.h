#ifndef __CLIENT_REQUEST__
#define __CLIENT_REQUEST__

#include <atomic>
#include <string>
#include <quickfix/Application.h>
#include <boost/flyweight.hpp>
using FlyweightStdString = boost::flyweights::flyweight<std::string>;

#include <memory>

enum class ClientRequestType {NEW_ORDER, CANCEL_ORDER};

class ClientRequest
{
    public:

        explicit ClientRequest(const std::string& csvRequest);
        ClientRequest(const ClientRequest&) ;// Added dummy copy ctor in order to use an atomic member

        inline bool requestSent() const { return m_requestSent.load(); }
        inline void markAsRequestSent() { m_requestSent.store(true);}
        void setOrderID(const std::string& orderID) { m_orderID = orderID;}
        void setSenderID(const std::string& senderID) { m_senderID = senderID; }

        ClientRequestType getType()const { return m_type; }
        const FIX::Side& getSide() const { return m_side; }
        const std::string& getOrderID() const { return m_orderID; }
        const std::string& getOrigOrderID() const {return m_origOrderID; }
        const std::string& getSymbol() const { return m_symbol; }
        long getQuantity() const { return m_quantity; }
        double getPrice() const { return m_price; }
        const std::string& getSenderID() const { return m_senderID; }
        const std::string& getTargetID() const { return m_targetID; }
        
    private:
        ClientRequestType m_type;
        FIX::Side m_side;
        std::string m_orderID;
        std::string m_origOrderID; // For cancel requests
        FlyweightStdString m_symbol;
        long m_quantity;
        double m_price;
        FlyweightStdString m_senderID;
        FlyweightStdString m_targetID;
        std::atomic<bool> m_requestSent;
};

using ClientRequestPtr = std::unique_ptr<ClientRequest>;

#endif