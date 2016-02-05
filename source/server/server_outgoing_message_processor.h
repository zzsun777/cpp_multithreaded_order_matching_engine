#ifndef __OUTGOING_MESSAGE_PROCESSOR__
#define __OUTGOING_MESSAGE_PROCESSOR__

#include <cassert>
#include <string>

#include <boost/format.hpp>

#include <quickfix/Session.h>
#include <quickfix/fix42/ExecutionReport.h>

#include <order_matcher/central_order_book.h>
#include <order_matcher/quickfix_converter.h>
#include <concurrent/actor.h>
#include <utility/logger.h>

using namespace concurrent;
using namespace order_matcher;

class OutgoingMessageProcessor : public Actor
{
    public:

        OutgoingMessageProcessor() : Actor("OutgoingWorker"), m_messageQueue(nullptr), m_execID(0)  
        // We can`t have more than 16 characters in Linux for a pthread name ,that is why compacted the thread name...
        {
        }
        
        void setMessageQueue(OutgoingMessageQueue* queue)
        {
            assert( queue != nullptr );
            m_messageQueue = queue;
        }

        void* run() override
        {
            LOG_INFO("Outgoing message processor", "Thread starting")

            // Let`s wait until message queue initialisation
            while (true)
            {
                if (isFinishing() == true)
                {
                    break;
                }

                if (m_messageQueue == nullptr)
                {
                    concurrent::Thread::sleep(1000);
                }
                else
                {
                    break;
                }
            }

            while (true)
            {
                if (isFinishing() == true)
                {
                    break;
                }
                
                OutgoingMessage message;
                if (m_messageQueue->dequeue(&message) == true)
                {
                    const Order& order = message.getOrder();

                    LOG_INFO("Outgoing message processor", boost::str(boost::format("Processing %s for order : %s ") % message.toString() % order.toString()) )
                        
                    FIX::TargetCompID targetCompID(order.getOwner());
                    FIX::SenderCompID senderCompID(order.getTarget());
                    auto status = convertToQuickFixOutgoingMessageType(message.getType());

                    FIX42::ExecutionReport fixOrder
                        (FIX::OrderID(order.getClientID()),
                        FIX::ExecID(genExecID()),
                        FIX::ExecTransType(FIX::ExecTransType_NEW),
                        FIX::ExecType(status),
                        FIX::OrdStatus(status),
                        FIX::Symbol(order.getSymbol()),
                        FIX::Side(order_matcher::convertOrderSideToQuickFix(order.getSide())),
                        FIX::LeavesQty(order.getOpenQuantity()),
                        FIX::CumQty(order.getExecutedQuantity()),
                        FIX::AvgPx(order.getAverageExecutedPrice()));

                    fixOrder.set(FIX::ClOrdID(order.getClientID()));
                    fixOrder.set(FIX::OrderQty(order.getQuantity()));

                    if (status == FIX::OrdStatus_FILLED ||
                        status == FIX::OrdStatus_PARTIALLY_FILLED)
                    {
                        fixOrder.set(FIX::LastShares(order.getLastExecutedQuantity()));
                        fixOrder.set(FIX::LastPx(order.getLastExecutedPrice()));
                    }

                    try
                    {
                        FIX::Session::sendToTarget(fixOrder, senderCompID, targetCompID);
                    }
                    catch (FIX::SessionNotFound&) 
                    {
                        // TO BE IMPLEMENTED
                    }
                        
                }
                else
                {
                    concurrent::Thread::yield();
                }
                
            }// while

            LOG_INFO("Outgoing message processor", "Thread exiting")
            return nullptr;
        }

    private:

        OutgoingMessageQueue* m_messageQueue = nullptr;
        int m_execID;

        std::string genExecID()
        {
            ++m_execID;
            return std::to_string(m_execID);
        }
};

#endif