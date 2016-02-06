//Coming from QuickFix
#ifdef _MSC_VER
#pragma warning( disable : 4503 4355 4786 )
#endif

#include "server.h"

#include <iostream>
#include <exception>
#include <algorithm>
#include <ctype.h>

#include <quickfix/FileStore.h>
#include <quickfix/SocketInitiator.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/Log.h>

#include <boost/format.hpp>

#include <order_matcher/central_order_book_visitor.h>
#include <order_matcher/order.h>
#include <order_matcher/quickfix_converter.h>
using namespace order_matcher;

#include <utility/file_utility.h>
#include <utility/pretty_exception.h>
#include <utility/logger.h>

Server::Server(const std::string& fixEngineConfigFile, bool pinThreadsToCores, int threadStackSize, bool hyperThreading, unsigned int queueSizePerThread, const std::vector<std::string>& symbols)
throw(std::runtime_error)
: m_fixEngineConfigFile{fixEngineConfigFile}
{
    if (!utility::doesFileExist(m_fixEngineConfigFile))
    {
        auto exceptionMessage = boost::str(boost::format("FIX configuration file %s does not exist") % m_fixEngineConfigFile);
        THROW_PRETTY_RUNTIME_EXCEPTION(exceptionMessage)
    }
   
    // Central order book initialisation
    concurrent::ThreadPoolArguments args;
    args.m_hyperThreading = hyperThreading;
    args.m_pinThreadsToCores = pinThreadsToCores;
    args.m_workQueueSizePerThread = queueSizePerThread;
    args.m_threadStackSize = threadStackSize;
    args.m_threadNames = symbols;
    m_centralOrderBook.initialise(args);

    // Attach central order book observer to the central order book
    m_centralOrderBook.attach(m_centralOrderBookObserver);

    // Incoming message dispatcher initialisation
    m_dispatcher.setCentralOrderBook(&m_centralOrderBook);
    m_dispatcher.start();

    // Outgoing message processor initialisation
    m_outgoingMessageProcessor.setMessageQueue(m_centralOrderBook.getOutgoingMessageQueue());
    m_outgoingMessageProcessor.start();
}

void Server::run()
{
    // FIX engine initialisation
    FIX::SessionSettings settings(m_fixEngineConfigFile);
    FIX::FileStoreFactory storeFactory(settings);
    FIX::ThreadedSocketAcceptor fixEngineAcceptor(*this, storeFactory, settings);
    // FIX engine start
    fixEngineAcceptor.start();    
    
    LOG_INFO("FIX Engine", "Acceptor started")

    displayUsage();

    // Engine loop
    while (true)
    {
        std::string value;
        std::cin >> value;

        std::transform(value.begin(), value.end(), value.begin(), ::tolower);

        if (value == "display")
        {
            CentralOrderBookVisitor visitor;
            m_centralOrderBook.accept(visitor);
            LOG_CONSOLE("FIX Engine", "All orders in the central order book :")
            LOG_CONSOLE("FIX Engine", "")
            LOG_CONSOLE("FIX Engine", visitor.toString())
        }
        else if (value == "quit")
        {
            LOG_INFO("FIX Engine", "Quit message received")
            break;
        }
        else
        {
            LOG_CONSOLE("FIX Engine", "Invalid user command")
            displayUsage();
        }

        std::cout << std::endl;
    }

    // FIX engine stop
    fixEngineAcceptor.stop(true);
    LOG_INFO("FIX Engine", "Acceptor stopped")
}

Server::~Server()
{
    m_outgoingMessageProcessor.shutdown();
    m_dispatcher.shutdown();
}

void Server::displayUsage()
{
    LOG_CONSOLE("FIX Engine", "")
    LOG_CONSOLE("FIX Engine", "Available commands :")
    LOG_CONSOLE("FIX Engine", "")
    LOG_CONSOLE("FIX Engine", "\tdisplay : Shows all order books in the central order book")
    LOG_CONSOLE("FIX Engine", "\tquit : Shutdowns the server")
    LOG_CONSOLE("FIX Engine", "")
}

void Server::onCreate(const FIX::SessionID& sessionID)
{
}

void Server::fromAdmin(const FIX::Message& message, const FIX::SessionID& sessionID) throw(FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon) 
{
}

void Server::toAdmin(FIX::Message& message, const FIX::SessionID& sessionID) 
{
}

void Server::onLogon( const FIX::SessionID& sessionID ) 
{
    LOG_INFO("FIX Engine", "New logon , session ID : " + sessionID.toString())
}

void Server::onLogout( const FIX::SessionID& sessionID ) 
{
    LOG_INFO("FIX Engine", "Logout , session ID : " + sessionID.toString())
}

void Server::toApp( FIX::Message& message, const FIX::SessionID& sessionID ) throw( FIX::DoNotSend ) 
{
    LOG_INFO("FIX Engine", "Sending fix message : " + message.toString() )
}

void Server::fromApp( const FIX::Message& message, const FIX::SessionID& sessionID )throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType )
{ 
    LOG_INFO("FIX Engine", "Receiving fix message : " + message.toString())
    crack(message, sessionID);
}

void Server::onMessage(const FIX42::NewOrderSingle& message, const FIX::SessionID& sessionID)
{
    LOG_INFO("FIX Engine", "New order message received :" + message.toString())

    FIX::SenderCompID senderCompID;
    FIX::TargetCompID targetCompID;
    FIX::ClOrdID clOrdID;
    FIX::Symbol symbol;
    FIX::Side side;
    FIX::OrdType ordType;
    FIX::Price price;
    FIX::OrderQty orderQty;

    // For the time being , we are ignoring TimeInForce

    message.getHeader().get(senderCompID);
    message.getHeader().get(targetCompID);
    message.get(clOrdID);
    message.get(symbol);
    message.get(side);
    message.get(ordType);
    message.get(orderQty);

    try
    {
        order_matcher::convertOrderTypeFromQuickFix(ordType);
        order_matcher::convertOrderSideFromQuickFix(side);
    }
    catch (std::runtime_error& e)
    {
        // Reject non supported order type or side
        Order rejectedOrder(clOrdID, symbol, senderCompID, targetCompID, OrderSide::BUY, OrderType::LIMIT, 0, 0);
        // Rather than pushing on to the dispatcher , directly push to the outgoing message processor via the central order book
        m_centralOrderBook.rejectOrder(rejectedOrder, e.what());
    }
    
    message.get(price);
    
    Order order(clOrdID, symbol, senderCompID, targetCompID, order_matcher::convertOrderSideFromQuickFix(side), order_matcher::convertOrderTypeFromQuickFix(ordType), price, (long) orderQty );
    IncomingMessage incomingMessage(order, order_matcher::IncomingMessageType::NEW_ORDER);
    m_dispatcher.pushMessage(incomingMessage);
}

void Server::onMessage(const FIX42::OrderCancelRequest& message, const FIX::SessionID&)
{
    LOG_INFO("FIX Engine", "Cancel order message received :" + message.toString())

    FIX::OrigClOrdID origClOrdID;
    FIX::Symbol symbol;
    FIX::Side side;
    FIX::SenderCompID senderCompID;
    FIX::ClOrdID clientID;

    message.getHeader().get(senderCompID);
    message.get(origClOrdID);
    message.get(clientID);
    message.get(symbol);
    message.get(side);

    Order order("", symbol, senderCompID, "", order_matcher::convertOrderSideFromQuickFix(side), order_matcher::OrderType::LIMIT, 0, 0);
    IncomingMessage incomingMessage(order, order_matcher::IncomingMessageType::CANCEL_ORDER, origClOrdID);
    m_dispatcher.pushMessage(incomingMessage);
}