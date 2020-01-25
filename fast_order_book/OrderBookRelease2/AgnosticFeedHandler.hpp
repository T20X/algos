#ifndef _FEED_HANDLER_HPP_
#define _FEED_HANDLER_HPP_

#include <ostream>

#include "SPSCBoundedQueue.hpp"
#include "Trade.hpp"
#include "FastOrderBook.hpp"
#include "Summary.hpp"
#include "Thread.hpp"
#include "UnifiedMsg.hpp"
#include "FastLogger.hpp"

class AgnosticFeedHandler : protected Thread
{
public:
    AgnosticFeedHandler(
        FastOrderBook& book, 
        Summary& stats,
        SPSCBoundedQueue<UnifiedMsg>& q,
        std::ostream& out,
        cpu_set_t* affinity,
        int bookDumpAtNThMsg = 10);

    ~AgnosticFeedHandler();

    void start();
    void stop();
    void tryToDumpBook();

private:
    void* process();

    typedef void (AgnosticFeedHandler::*msgHandler)(const UnifiedMsg* msg);

    void onNewOrder(const UnifiedMsg* msg);
    void onModifyOrder(const UnifiedMsg* msg);
    void onDelOrder(const UnifiedMsg* msg);
    void onTrade(const UnifiedMsg* msg);

    void dumpTopMid();

private:
    FastOrderBook& m_book;
    Summary& m_stats;
    SPSCBoundedQueue<UnifiedMsg>& m_q;
    std::ostream& m_out;
    int m_bookDumpAtNThMsg;

    double m_lastTradePrice;
    uint64_t m_totVolAtLastTradePrice;    

    size_t m_expectedTradeNum;

    int m_seqNum;

    FastLogger m_logger;

    std::vector<msgHandler> m_handlers;
};

#endif // _FEED_HANDLER_HPP_
