#ifndef _FAST_LOGGER_HPP_
#define _FAST_LOGGER_HPP_

#include <ostream>
#include <iostream>
#include <utility>
#include <iomanip>

#include "SPSCBoundedQueue.hpp"
#include "Thread.hpp"

class FastLogger : protected Thread
{
public:
    FastLogger(std::ostream& out)
        : Thread("Fast Logger", 0/*default affinity*/),
          m_q(FAST_LOGGER_MSG_Q_SIZE),
          m_out(out) 
    {
    }
    
    void start()
    {
        Thread::create();
    }

    void stop()
    {
        sleep(2);
        Thread::cancel();
    }

    struct PriceLevel
    {
        double bidPrice;
        double askPrice;
        
        uint32_t bidQty;
        uint32_t askQty;
    };

    FastLogger& operator<<(const PriceLevel& val) 
    {
        LogMsg* msg = 0;
        m_q.getForWrite(&msg);
        msg->type = LogMsg::PRICE_LEVEL;
        msg->val.levelVal = val;
        m_q.notifyPush();
        return *this;
    }
 

    FastLogger& operator<<(const char* val) 
    {
        LogMsg* msg = 0;
        m_q.getForWrite(&msg);
        msg->type = LogMsg::STATIC_STRING;
        msg->val.staticString = val;
        m_q.notifyPush();
        return *this;
    }

    FastLogger& operator<<(double val) 
    { 
        LogMsg* msg = 0;
        m_q.getForWrite(&msg);
        msg->type = LogMsg::DOUBLE;
        msg->val.doubleVal = val;
        m_q.notifyPush();
        return *this;
    }

    FastLogger& operator<<(uint64_t val) 
    {
        LogMsg* msg = 0;
        m_q.getForWrite(&msg);
        msg->type = LogMsg::UINT64;
        msg->val.uint64_tVal = val;
        m_q.notifyPush();
        return *this;
    }

    FastLogger& operator<<(uint32_t val) 
    {
        LogMsg* msg = 0;
        m_q.getForWrite(&msg);
        msg->type = LogMsg::UINT;
        msg->val.intVal = val;
        m_q.notifyPush();
        return *this;
    }

    FastLogger& operator<<(char val) 
    {
        LogMsg* msg = 0;
        m_q.getForWrite(&msg);
        msg->type = LogMsg::CHAR;
        msg->val.charVal = val;
        m_q.notifyPush();
        return *this;
    }

private:
    void* process()
    {
        while(1)
        {
            LogMsg* msg = 0;
            m_q.getForRead(&msg);
            if (msg == 0) { continue; }
            
            switch(msg->type) 
            {
                case LogMsg::PRICE_LEVEL:
                {
                    PriceLevel& level = msg->val.levelVal;

                    if (level.bidPrice != INVALID_PRICE)
                    {            
                        m_out << "\n" << std::setprecision(OSTREAM_PRECISION) 
                              << std::setw(OSTREAM_QTY_WIDTH) << level.bidQty  << " @ "
                              << std::setw(OSTREAM_PRICE_WIDTH) << level.bidPrice << " | ";
                    }        
                    else
                    {
                        m_out << "\n" << std::setw(OSTREAM_QTY_WIDTH + 
                                                   OSTREAM_PRICE_WIDTH +
                                                   6/*magic*/) << " | ";
                    }

                    if (level.askPrice != INVALID_PRICE)
                    {        
                        m_out << std::setprecision(OSTREAM_PRECISION) 
                              << std::setw(OSTREAM_QTY_WIDTH) << level.askQty<< " @ "
                              << std::setw(OSTREAM_PRICE_WIDTH) << level.askPrice << " | ";
                    }    

                    break;
                }
                case LogMsg::STATIC_STRING:
                {
                    m_out << msg->val.staticString;
                    break;
                }
                case LogMsg::DOUBLE:
                {
                    m_out << std::setprecision(OSTREAM_PRECISION);
                    m_out << msg->val.doubleVal;
                    break;
                }
                case LogMsg::UINT64:
                {
                    m_out << msg->val.uint64_tVal;
                    break;
                }
                case LogMsg::UINT:
                {
                    m_out << msg->val.intVal;
                    break;
                }
                case LogMsg::CHAR:
                {
                    m_out << msg->val.charVal;
                    break;
                }

                default:
                {
                    m_out << "UnknownType";
                }
            }

            m_q.notifyPop();
        }

        return 0;
    }

private:
    FastLogger(const FastLogger& antother);
    FastLogger& operator==(const FastLogger& antother);

private:
    struct LogMsg
    {
        enum LogItemType
        {
            PRICE_LEVEL,
            STATIC_STRING,
            DOUBLE,
            UINT64,
            UINT,
            CHAR
        };

        char busy;
        LogItemType type;

        union LogItem
        {
            PriceLevel levelVal;
            const char* staticString;
            double doubleVal;
            uint64_t uint64_tVal;
            uint32_t intVal;
            char charVal;
        } val;
    };

    SPSCBoundedQueue<LogMsg> m_q;
    std::ostream& m_out;
};

#endif // _FAST_LOGGER_HPP_

