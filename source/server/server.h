#ifndef _SERVER_H_
#define _SERVER_H_

#include <string>
#include <vector>
#include <memory>

#include <boost/noncopyable.hpp>

#include <order_matcher/central_order_book.h>
#include <order_matcher/central_order_book_observer.h>

#include "server_outgoing_message_processor.h"
#include "server_incoming_message_dispatcher.h"

#include <quickfix/Application.h>
#include <quickfix/MessageCracker.h>
#include <quickfix/ThreadedSocketAcceptor.h>

// FIX 4.2 specific 
#include <quickfix/fix42//NewOrderSingle.h>
#include <quickfix/fix42/OrderCancelRequest.h>

class Server : public boost::noncopyable, public FIX::Application, public FIX::MessageCracker
{
    public:
    
        Server(const std::string& fixEngineConfigFile, bool pinThreadsToCores, int threadStackSize, bool hyperThreading, unsigned int queueSizePerThread, const std::vector<std::string>& symbols) throw(std::runtime_error);
        ~Server();
        void run();

    private:
        void displayUsage();
        // FIX Engine Application overloads
        void onCreate(const FIX::SessionID&) override;
        void toAdmin(FIX::Message&, const FIX::SessionID&) override;
        void fromAdmin(const FIX::Message&, const FIX::SessionID&) throw(FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon) override;
        void onLogon(const FIX::SessionID& sessionID) override;
        void onLogout(const FIX::SessionID& sessionID) override;
        void toApp(FIX::Message&, const FIX::SessionID&) throw(FIX::DoNotSend) override;
        void fromApp(const FIX::Message& message, const FIX::SessionID& sessionID) throw(FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType) override;

        // FIX Engine MessageCracker overloads
        void onMessage(const FIX42::NewOrderSingle&, const FIX::SessionID&);
        void onMessage(const FIX42::OrderCancelRequest&, const FIX::SessionID&);

        std::string m_fixEngineConfigFile;
        order_matcher::CentralOrderBook m_centralOrderBook;
        order_matcher::CentralOrderBookObserver m_centralOrderBookObserver;
        OutgoingMessageProcessor m_outgoingMessageProcessor;
        IncomingMessageDispatcher m_dispatcher;
};

#endif
