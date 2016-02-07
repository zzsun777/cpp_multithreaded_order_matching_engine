#ifndef _CLIENT_APPLICATION_H_
#define _CLIENT_APPLICATION_H_

#include <boost/noncopyable.hpp>

#include <quickfix/Application.h>
#include <quickfix/MessageCracker.h>
#include <quickfix/Values.h>
#include <quickfix/Mutex.h>

#include <quickfix/fix42/NewOrderSingle.h>
#include <quickfix/fix42/OrderCancelRequest.h>
#include <quickfix/fix42/ExecutionReport.h>

#include "client_request.h"

#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <atomic>
#include <thread>
#include <mutex>

class ClientApplication : public FIX::Application, public FIX::MessageCracker, public boost::noncopyable
{
    public:
        ClientApplication(const std::string& csvTestFile, const std::string& fixEngineConfigFile);
        ~ClientApplication();
        void run();
    private:
        void loadRequests(const std::string& csvTestFile);
        void executeRequests();
        void waitTillSessionStarts();
        void waitTillAllRequestsComplete();
        std::string newOrder(const std::string& orderID, const std::string& symbol, const FIX::Side& side, double price, long quantity, const std::string& senderID, const std::string& targetID);
        std::string  cancelOrder(const std::string& orderID, const std::string& clOrigOrderId, const std::string& symbol, const FIX::Side& side, const std::string& senderID, const std::string& targetID);

        inline bool sessionAlive() const
        {
            return m_sessionAlive;
        }

        inline void markRequestAsSent(const std::string& orderID)
        {
            for (auto& request : requests)
            {
                if (request->getOrderID().compare(orderID) == 0)
                {
                    request->markAsRequestSent();
                }
            }
        }

        inline void markCancelRequestAsSent(const std::string& orderID)
        {
            for (auto& request : requests)
            {
                if (request->getType() == ClientRequestType::CANCEL_ORDER && request->getOrigOrderID().compare(orderID)==0)
                {
                    request->markAsRequestSent();
                }
            }
        }

        inline bool allRequestsSent() const
        {
            for (auto& request : requests)
            {
                if (request->requestSent() == false)
                {
                    return false;
                }
            }
            return true;
        }

        void onCreate(const FIX::SessionID&) override {}
        void onLogon(const FIX::SessionID& sessionID) override;
        void onLogout(const FIX::SessionID& sessionID) override;
        void toAdmin(FIX::Message&, const FIX::SessionID&) override{}
        void toApp(FIX::Message&, const FIX::SessionID&) throw(FIX::DoNotSend) override;
        void fromAdmin(const FIX::Message&, const FIX::SessionID&) throw(FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon) override {}
        void fromApp(const FIX::Message& message, const FIX::SessionID& sessionID) throw(FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType) override;
        void onMessage(const FIX42::ExecutionReport&, const FIX::SessionID&) override;

        int m_orderID = 0;
        std::string m_fixEngineConfigFile = "";
        std::string m_ClientID;
        bool m_sessionAlive = false;

        std::vector<ClientRequestPtr> requests;

        void threadSafeConsoleOut(const std::string& message);
        void consoleOutputThread();
        void dumpMessageBufferToFile();
        std::thread spawn() { return std::thread([this] { consoleOutputThread(); });}
        std::thread m_consoleOutputThread;
        std::mutex m_consoleOutputLock;
        std::vector<std::string> m_consoleMessages;
        std::stringstream m_fileOutputMessageBuffer;
        std::atomic<bool> m_applicationEnding;

        inline const std::string generateOrderID()
        {
            ++m_orderID;
            return std::to_string(m_orderID);
        }

        std::string orderStatusToString(const FIX::OrdStatus& type);
};

#endif