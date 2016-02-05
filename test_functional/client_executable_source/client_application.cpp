#include "client_application.h"
#include <quickfix/Session.h>
#include <iostream>
#include <fstream>
using namespace std;

#include <utility/pretty_exception.h>
#include <utility/utility.h>
#include <concurrent/thread.h>

#include <boost/format.hpp>

#include <quickfix/FileStore.h>
#include <quickfix/SocketInitiator.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/Log.h>

#define ONE_SECOND 1000
#define SESSION_TIMEOUT 120
#define SERVER_PROCESS_TIMEOUT 1200

ClientApplication::ClientApplication(const string& csvTestFile, const string& fixEngineConfigFile) : m_fixEngineConfigFile(fixEngineConfigFile)
{
    //////////////////////////////////////////
    // Check if config and test files exist

    if (!utility::doesFileExist(m_fixEngineConfigFile))
    {
        auto exceptionMessage = boost::str(boost::format("FIX configuration file %s does not exist") % m_fixEngineConfigFile);
        THROW_PRETTY_EXCEPTION(exceptionMessage)
    }

    if (!utility::doesFileExist(csvTestFile))
    {
        auto exceptionMessage = boost::str(boost::format("Test file %s does not exist") % csvTestFile);
        THROW_PRETTY_EXCEPTION(exceptionMessage)
    }

    // Extract client id from the name of the cfg file
    m_ClientID = m_fixEngineConfigFile.substr(0, m_fixEngineConfigFile.length() - 4);

    // Load requests from csv file
    loadRequests(csvTestFile);

    m_consoleOutputThread = spawn();

    m_applicationEnding.store(false);
}

ClientApplication::~ClientApplication()
{
    m_applicationEnding.store(true);
    m_consoleOutputThread.join();
}

// COARSE-GRAINED LOCKING AS IT IS ONLY FOR TEST SOFTWARE
void ClientApplication::consoleOutputThread()
{
    while (m_applicationEnding.load() == false )
    {
        { 
            // According to the standards guard object should be destroyed
            // in each iteration , however this is not what happened with GCC 4.8 on CentOS7
            // and caused a dead lock...
            // Therefore explicitly creating a scope
            std::lock_guard<std::mutex> guard(m_consoleOutputLock);

            for (auto message : m_consoleMessages)
            {
                std::cout << message << endl;

                m_fileOutputMessageBuffer << utility::getCurrentDateTime();
                m_fileOutputMessageBuffer << " : "<< message << endl;
            }

            m_consoleMessages.clear();

        }
        concurrent::Thread::sleep(50);
        
    }
}

void ClientApplication::threadSafeConsoleOut(const std::string& message)
{
    std::lock_guard<std::mutex> guard(m_consoleOutputLock);
    m_consoleMessages.push_back(message);
}

void ClientApplication::run()
{
    // Initialise FIX engine
    FIX::SessionSettings settings(m_fixEngineConfigFile);
    FIX::FileStoreFactory storeFactory(settings);
    FIX::SocketInitiator initiator(*this, storeFactory, settings);

    // Start FIX engine
    initiator.start();

    threadSafeConsoleOut("FIX Engine started");

    try
    {
        threadSafeConsoleOut("Trying to connect to the server");
        waitTillSessionStarts();

        threadSafeConsoleOut("Session to the server started");
        threadSafeConsoleOut("Now executing all requests");

        executeRequests();

        waitTillAllRequestsComplete();
        threadSafeConsoleOut("All requests are processed by the server");
    }
    catch (FIX::SessionNotFound&)
    {
        std::cout << "FIX session not found..." << endl;
    }

    initiator.stop();
    threadSafeConsoleOut("FIX Engine stopping");
    dumpMessageBufferToFile();
}

void ClientApplication::dumpMessageBufferToFile()
{
    string logName = m_ClientID + ".txt";
    if ( utility::doesFileExist(logName))
    {
        utility::deleteFile(logName);
    }

    ofstream logFile(logName);

    if (logFile.is_open())
    {
        logFile << m_fileOutputMessageBuffer.str();
        logFile.close();
    }
    else
    {
        cerr << "Log output can`t be created";
    }
}

void ClientApplication::waitTillAllRequestsComplete()
{
    unsigned int secondsWaited = 0;

    while (allRequestsSent() == false)
    {
        concurrent::Thread::sleep(ONE_SECOND);
        ++secondsWaited;

        if (secondsWaited >= SERVER_PROCESS_TIMEOUT)
        {
            THROW_PRETTY_EXCEPTION(std::string("Timeout , when waiting responses from the server , check the server logs"))
        }
    }
}

void ClientApplication::waitTillSessionStarts()
{
    unsigned int secondsWaited = 0;

    while (sessionAlive() == false)
    {
        concurrent::Thread::sleep(ONE_SECOND);
        ++secondsWaited;

        if (secondsWaited >= SESSION_TIMEOUT)
        {
            THROW_PRETTY_EXCEPTION(std::string("Timeout , could not establish a session with the server"))
        }
    }
}

void ClientApplication::loadRequests(const std::string& csvTestFile)
{
    // For each line in the test file add a new client request
    ifstream file(csvTestFile);
    if (!file.good())
    {
        auto exceptionMessage = boost::str(boost::format("File %s could not be opened") % csvTestFile);
        THROW_PRETTY_EXCEPTION(exceptionMessage)
    }

    file.seekg(0, std::ios::beg);
    string line;

    while (std::getline(file, line))
    {
        auto lineLength = line.length();

        if (line.c_str()[0] == '#' || lineLength == 0) // Skip comment lines and empty lines
        {
            continue;
        }

        ClientRequestPtr request( new ClientRequest(line) );
        requests.push_back(std::move(request));
        requests[requests.size() - 1]->setSenderID(m_ClientID);
    }
    file.close();
}

void ClientApplication::executeRequests()
{
    for (auto& request : requests)
    {
        auto orderID = generateOrderID();
        request->setOrderID(orderID);        

        if (request->getType() == ClientRequestType::NEW_ORDER)
        {
            newOrder(request->getOrderID(), request->getSymbol(), request->getSide(), request->getPrice(), request->getQuantity(), request->getSenderID(), request->getTargetID());
        }
        else
        {
            cancelOrder(request->getOrderID(), request->getOrigOrderID(), request->getSymbol(), request->getSide(), request->getSenderID(), request->getTargetID());
        }
    }
}

void ClientApplication::onLogon( const FIX::SessionID& sessionID )
{
    m_sessionAlive = true;
    threadSafeConsoleOut("FIX Engine logon, session id : " + sessionID.toString() );
}

void ClientApplication::onLogout( const FIX::SessionID& sessionID )
{
    m_sessionAlive = false;
    threadSafeConsoleOut("FIX Engine logout, session id : " + sessionID.toString());
}

std::string ClientApplication::orderStatusToString(const FIX::OrdStatus& type)
{
    switch (type)
    {
        case FIX::OrdStatus_FILLED:
            return "FILLED";
        break;
        case FIX::OrdStatus_PARTIALLY_FILLED:
            return "PARTIALLY FILLED";
        case FIX::OrdStatus_CANCELED:
            return "CANCELED";
        case FIX::OrdStatus_NEW :
            return "ACCEPTED BY THE SERVER";
        case FIX::OrdStatus_REJECTED:
            return "REJECTED BY THE SERVER";
        default:
            THROW_PRETTY_EXCEPTION(string("Not supported order status type") )
    }
}

void ClientApplication::fromApp( const FIX::Message& message, const FIX::SessionID& sessionID ) throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType )
{
    crack( message, sessionID );
    //threadSafeConsoleOut("FIX Engine message received : " + message.toString());
}

void ClientApplication::toApp( FIX::Message& message, const FIX::SessionID& sessionID ) throw( FIX::DoNotSend )
{
    try
    {
        FIX::PossDupFlag possDupFlag;
        message.getHeader().getField( possDupFlag );
        if ( possDupFlag ) throw FIX::DoNotSend();
    }
    catch ( FIX::FieldNotFound& ) 
    {
    }

    //threadSafeConsoleOut("FIX Engine message sent : " + message.toString());
}

void ClientApplication::onMessage( const FIX42::ExecutionReport& report, const FIX::SessionID& ) 
{
    FIX::ClOrdID clOrdID;
    report.get(clOrdID);

    FIX::OrdStatus orderStatus;
    report.get(orderStatus);
    
    string clientOrderID = clOrdID.getString();
    string status =  orderStatusToString(orderStatus);

    threadSafeConsoleOut(boost::str(boost::format("Execution report from server for client order ID : %s , status : %s ") % clientOrderID % status ));

    if ( orderStatus == FIX::OrdStatus_FILLED || orderStatus == FIX::OrdStatus_REJECTED)
    { 
        markRequestAsSent(clientOrderID);
    }

    if (orderStatus == FIX::OrdStatus_CANCELED)
    {
        // First we mark the order being canceled 
        markRequestAsSent(clientOrderID);
        // Then we have to mark the cancel request itself
        markCancelRequestAsSent(clientOrderID);
    }
}

string ClientApplication::newOrder(const std::string& orderID, const string& symbol, const FIX::Side& side, double price, long quantity, const string& senderID, const string& targetID)
{
    FIX42::NewOrderSingle newOrderSingle(
        FIX::ClOrdID(orderID), FIX::HandlInst('1'), FIX::Symbol(symbol), side,
        FIX::TransactTime(), FIX::OrdType(FIX::OrdType_LIMIT));

    newOrderSingle.set(FIX::OrderQty(quantity));
    newOrderSingle.set(FIX::TimeInForce(FIX::TimeInForce_DAY));
    newOrderSingle.set(FIX::Price(price));

    FIX::Header& header = newOrderSingle.getHeader();

    header.setField(FIX::SenderCompID(senderID));
    header.setField(FIX::TargetCompID(targetID));

    FIX::Session::sendToTarget(newOrderSingle);

    return orderID;
}

string ClientApplication::cancelOrder(const std::string& orderID, const string& clOrigOrderId, const string& symbol, const FIX::Side& side, const std::string& senderID, const std::string& targetID)
{

    FIX42::OrderCancelRequest orderCancelRequest(FIX::OrigClOrdID(clOrigOrderId),
        FIX::ClOrdID(orderID), FIX::Symbol(symbol), side, FIX::TransactTime());

    FIX::Header& header = orderCancelRequest.getHeader();

    header.setField(FIX::SenderCompID(senderID));
    header.setField(FIX::TargetCompID(targetID));

    FIX::Session::sendToTarget(orderCancelRequest);

    return orderID;
}