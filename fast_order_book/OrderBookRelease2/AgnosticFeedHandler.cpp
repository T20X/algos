#include <iterator>
#include <algorithm>

#include "AgnosticFeedHandler.hpp"
#include "LatencyMonitor.hpp"

/*************************************************************
 */
AgnosticFeedHandler::AgnosticFeedHandler(
    FastOrderBook& book, 
    Summary& stats, 
    SPSCBoundedQueue<UnifiedMsg>& q,
    std::ostream& out,
    cpu_set_t* affinity,
    int bookDumpAtNThMsg)
    : Thread("AgnosticFeedHandler Thread", affinity),
      m_book(book),
      m_stats(stats),
      m_q(q),
      m_out(out), 
      m_bookDumpAtNThMsg(bookDumpAtNThMsg),
      m_lastTradePrice(INVALID_PRICE), 
      m_totVolAtLastTradePrice(0),
      m_expectedTradeNum(0),
      m_seqNum(0),
      m_logger(out)
{
    m_handlers.resize(UnifiedMsg::MSG_NUM);
    m_handlers[UnifiedMsg::ORDER_NEW_MSG] = &AgnosticFeedHandler::onNewOrder;
    m_handlers[UnifiedMsg::ORDER_MODIFY_MSG] = &AgnosticFeedHandler::onModifyOrder;
    m_handlers[UnifiedMsg::ORDER_DELETE_MSG] = &AgnosticFeedHandler::onDelOrder;
    m_handlers[UnifiedMsg::TRADE_MSG] = &AgnosticFeedHandler::onTrade; 
}

/*************************************************************
 */
AgnosticFeedHandler::~AgnosticFeedHandler()
{
}

/*************************************************************
 */
void AgnosticFeedHandler::start()
{
    m_book.warmup();
    m_logger.start();
    Thread::create();
}

/*************************************************************
 */
void AgnosticFeedHandler::stop()
{
    Thread::cancel();
    m_logger.stop();
}

/*************************************************************
 */
void* AgnosticFeedHandler::process()
{
    UnifiedMsg* msg = 0;

    while(1)
    {
        m_q.getForRead(&msg);
        if (msg == 0) continue;

        try
        {
            (this->*m_handlers[msg->type])(msg); 
            LatencyMonitor::instance().end("Avg Latency", msg->timestamp);
        }
        catch(const std::exception& e)
        {
            std::cerr << "\nException while processing message: [" 
                      << e.what() << "]";
        }
        catch(...)
        {
            std::cerr << "\nUnknown exception while processing message";
        }

        m_q.notifyPop();
    }

    return 0;
}

/*************************************************************
 */
void AgnosticFeedHandler::onNewOrder(const UnifiedMsg* msg) 
{
    const OrderAdd& newBorn = msg->agnosticMsg.orderA; 
    if (unlikely(m_book.insertOrder(newBorn)))
    {
        m_stats.m_dupAddOrderIdNum++;
    }
    else
    {
        //
        // only new orders can trigger trades
        //

        if (m_book.isCrossed())
        {
            if (m_expectedTradeNum)
            {
                //we were expecting some trades but instead got a new book crossing;
                m_book.clearExpectedTrade(m_expectedTradeNum);
            }            

            //we expect some of following messages to be a trade
            //before the top of the book crosses again
            m_expectedTradeNum = m_book.howManyTradesExpected();

            //to fix an edge case in case when the last
            //exchange message generated a trade, but it never came
            m_stats.m_matchButNoTradeNum++; 
        }
        
        dumpTopMid();    
    }

    tryToDumpBook();
}

/*************************************************************
 */
void AgnosticFeedHandler::onModifyOrder(const UnifiedMsg* msg) 
{
    const OrderModify& modified = msg->agnosticMsg.orderM; 
    m_book.modifyOrder(modified);
    tryToDumpBook();
}

/*************************************************************
 */
void AgnosticFeedHandler::onDelOrder(const UnifiedMsg* msg) 
{
    const OrderDel& old = msg->agnosticMsg.orderD; 
    if (unlikely(m_book.deleteOrder(old)))
    {
        m_stats.m_delWithNoOrderNum++;
    }
    else
    {
        dumpTopMid();
    }    
    
    tryToDumpBook();
}

/*************************************************************
 */
void AgnosticFeedHandler::onTrade(const UnifiedMsg* msg) 
{
    const Trade& t = msg->agnosticMsg.trade;
    if (unlikely(!m_book.match(t)))
    {
        m_stats.m_tradeWithNoOrderNum++;
        return;
    }
    
    if (likely(m_expectedTradeNum/*any good trade*/)) 
    {        
        m_expectedTradeNum--;
    }
    if (!m_expectedTradeNum)
    {
        //to fix an edge case in case when the last
        //exchange message generated a trade, but it never came
        m_stats.m_matchButNoTradeNum--; 
    }

    if (isPriceEqual(t.price, m_lastTradePrice))    
    {
        m_totVolAtLastTradePrice += t.qty;    
    }
    else    
    {
        m_totVolAtLastTradePrice = t.qty;
        m_lastTradePrice = t.price;
    }

    m_logger << "\nT," 
             << t.qty << "," 
             << t.price << " => " 
             << m_totVolAtLastTradePrice << "@"
             << m_lastTradePrice;    
}

/*************************************************************
 */
void AgnosticFeedHandler::tryToDumpBook()
{
    if (unlikely(++m_seqNum % m_bookDumpAtNThMsg == 0))
        m_book.dumpToHuman(m_logger);
}

/*************************************************************
 */
void AgnosticFeedHandler::dumpTopMid() 
{
    double topMid = m_book.getTopMid();
    if (unlikely(topMid == INVALID_PRICE))
    {
        m_logger << "\n" << "NAN";
    }
    else
    {
        m_logger << "\n" << topMid;
    }    
}

