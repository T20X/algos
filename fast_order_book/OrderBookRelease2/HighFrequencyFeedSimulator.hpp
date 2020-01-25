#ifndef _NEW_ORDER_GENERATOR_HPP_
#define _NEW_ORDER_GENERATOR_HPP_

#include <cstdlib>
#include <algorithm>
#include <time.h>

#include "Order.hpp"
#include "Common.hpp"
#include "Thread.hpp"

class HighFrequencyFeedSimulator : protected Thread
{
public:
    HighFrequencyFeedSimulator(
        SPSCBoundedQueue<UnifiedMsg>& q, 
        cpu_set_t* affinity, int N)
           : Thread("Simulator Thread", affinity),
             m_q(q),
             m_n(N) 
    {}

    void start()
    {
        Thread::create();
        Thread::join();
    }

    void* process()
    {
        srand ( time(NULL) );
        uint64_t timestamp;
        for (int i = 0, j =0; i < m_n; i++, j++)
        {
            double price =  (double(rand() % 10)) * (double(rand() % 10000) / 10000.0) + 99.9; 
            char side = (i < m_n/2 ? 'S' : 'B');
            uint32_t qty = 10;
        
            if (i % 3 == 0)
            { 
                OrderModify o;
                o.id = i;
                o.side = side;
                o.qty = qty;
                o.price = price;

                m_ordersToModify.push_back(o);
            }

            if (i % 3 == 0)
            { 
                OrderDel o;
                o.id = i;
                o.side = side;
                o.qty = qty;
                o.price = price;

                m_ordersToDelete.push_back(o);
            }

            LatencyMonitor::instance().begin("Avg Latency", timestamp);

            UnifiedMsg* newBorn = 0;
            m_q.getForWrite(&newBorn);
            newBorn->agnosticMsg.orderA.side = side;
            newBorn->agnosticMsg.orderA.id = i;
            newBorn->agnosticMsg.orderA.qty = qty;
            newBorn->agnosticMsg.orderA.price = price;
            newBorn->type = UnifiedMsg::ORDER_NEW_MSG;
            newBorn->timestamp = timestamp;

            m_q.notifyPush(); 
        }
        
        std::random_shuffle(m_ordersToModify.begin(), m_ordersToModify.end());
        for (OrderContainer::const_iterator it = m_ordersToModify.begin(); 
             it != m_ordersToModify.end(); 
             ++it)
        {
            LatencyMonitor::instance().begin("Avg Latency", timestamp);

            UnifiedMsg* modified = 0;
            m_q.getForWrite(&modified);
            modified->agnosticMsg.orderM.side = it->side;
            modified->agnosticMsg.orderM.qty = 3;
            modified->agnosticMsg.orderM.price = it->price;
            modified->agnosticMsg.orderM.id = it->id;
            modified->type = UnifiedMsg::ORDER_MODIFY_MSG;
            modified->timestamp = timestamp;
            m_q.notifyPush();
        }

        std::random_shuffle(m_ordersToDelete.begin(), m_ordersToDelete.end());
        for (OrderContainer::const_iterator it = m_ordersToDelete.begin(); 
             it != m_ordersToDelete.end(); 
             ++it)
        {
            LatencyMonitor::instance().begin("Avg Latency", timestamp);

            UnifiedMsg* old = 0;
            m_q.getForWrite(&old);
            old->agnosticMsg.orderD.side = it->side;
            old->agnosticMsg.orderD.qty = 3;
            old->agnosticMsg.orderD.price = it->price;
            old->agnosticMsg.orderD.id =  it->id;
            old->type = UnifiedMsg::ORDER_DELETE_MSG;
            old->timestamp = timestamp;
            m_q.notifyPush();
        } 
    }   

private:
    SPSCBoundedQueue<UnifiedMsg>& m_q;
    int m_n;

    typedef std::vector<Order> OrderContainer;
    OrderContainer m_ordersToModify;
    OrderContainer m_ordersToDelete;

};

#endif // _NEW_ORDER_GENERATOR_HPP_

