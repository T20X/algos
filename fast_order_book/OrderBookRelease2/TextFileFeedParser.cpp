#include <iostream>
#include <limits>
#include <climits>
#include <fstream>
#include <string.h>
#include <stdlib.h>

#include "TextFileFeedParser.hpp"
#include "LatencyMonitor.hpp"
#include "UnifiedMsg.hpp"
#include "Common.hpp"

/*************************************************************
 */
TextFileFeedParser::TextFileFeedParser(
    SPSCBoundedQueue<UnifiedMsg>& q,
    const std::string& filename,
    cpu_set_t* affinity,
    Summary& stats)
      : Thread("TextFileFeedParser thread", affinity),
        m_q(q),
        m_filename(filename),
        m_stats(stats)
{
    m_parsers.resize(256, &TextFileFeedParser::reportCorruptedMsg);
    m_parsers['A'] = &TextFileFeedParser::parseN_Order;
    m_parsers['X'] = &TextFileFeedParser::parseD_Order;
    m_parsers['M'] = &TextFileFeedParser::parseM_Order;
    m_parsers['T'] = &TextFileFeedParser::parseTrade;
}

/*************************************************************
 */
TextFileFeedParser::~TextFileFeedParser()
{
}

/*************************************************************
 */
void TextFileFeedParser::start()
{
    Thread::create();
    Thread::join();
}

/************************************************************
*/
void* TextFileFeedParser::process()
{
    std::ifstream feed;

    try
    {    
        char buff[MAX_MSG_SIZE];    

        feed.exceptions(std::ifstream::badbit);
        feed.open(m_filename.c_str(), std::ios::in);    

        /* Would be nice to have another thread for
           loading the file, but I am out of time */
    
        while (!feed.eof() && feed.good())
        {
            BEGIN_LATENCY_TRACE("Get Line From File") 
            feed.getline(buff, MAX_MSG_SIZE);
            END_LATENCY_TRACE("Get Line From File") 

            parseMsg(buff);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "\nException while processing file feed: [" 
                  << e.what() << "]";
        feed.close();
    }
    
    std::cout << "Parse of " << m_filename.c_str() 
              << " completed" << std::endl;

    feed.close();
    return 0;
}

/************************************************************
 */
void TextFileFeedParser::parseMsg(char* raw)
{
    uint64_t timestamp;
    LatencyMonitor::instance().begin("Avg Latency", timestamp);

    while(isspace(*raw)) raw++; 
    if (raw[0] == 0 || raw[0] == '\n')
    {
        //Got an emty string
        return;
    }

    UnifiedMsg* msg = 0;
    ParseFunc parser = m_parsers[raw[0]];
    if (likely((this->*parser)(raw, &msg)))
    {
        msg->timestamp = timestamp;
        m_q.notifyPush();
    }
}

/*************************************************************
 */
bool TextFileFeedParser::parseN_Order(char* raw, UnifiedMsg** msg)
{
    m_q.getForWrite(msg);
    if (likely(doParseOrder(++raw, (*msg)->agnosticMsg.orderA)))
    {
        (*msg)->type = UnifiedMsg::ORDER_NEW_MSG;
        return true;
    } 

    return false;
}

/*************************************************************
 */
bool TextFileFeedParser::parseM_Order(char* raw, UnifiedMsg** msg)
{
    m_q.getForWrite(msg);
    if (likely(doParseOrder(++raw, (*msg)->agnosticMsg.orderM)))
    {
        (*msg)->type = UnifiedMsg::ORDER_MODIFY_MSG;
        return true;
    }

    return false;
}

/*************************************************************
 */
bool TextFileFeedParser::parseD_Order(char* raw, UnifiedMsg** msg)
{
    m_q.getForWrite(msg);
    if (likely(doParseOrder(++raw, (*msg)->agnosticMsg.orderD)))
    {
        (*msg)->type = UnifiedMsg::ORDER_DELETE_MSG;
        return true;
    }

    return false; 
}

/*************************************************************
 */
bool TextFileFeedParser::parseTrade(char* raw, UnifiedMsg** msg)
{
    m_q.getForWrite(msg);
    if (likely(doParseTrade(++raw, (*msg)->agnosticMsg.trade)))
    {
        (*msg)->type = UnifiedMsg::TRADE_MSG;
        return true;
    } 

    return false;
}

/*************************************************************
 */
bool TextFileFeedParser::reportCorruptedMsg(char* raw, UnifiedMsg** msg)
{
    m_stats.m_corruptedMsgNum++;
    return false;
}

/*************************************************************
 */
bool TextFileFeedParser::doParseOrder(char* msg, Order& o)
{
    BEGIN_LATENCY_TRACE("Parse Order") 
    size_t from = 0, to = 0;

    //Id

    from = ++to; // skip ','
    for (; msg[to] != ','; to++)
    {
        if(unlikely(msg[to] == '\0'))
        {
            m_stats.m_corruptedMsgNum++;        
            return false;
        }
    } 

    msg[to] = '\0';
    o.id = strToPositiveUnsInt(msg+from);
    if (unlikely(o.id == 0)) 
        return false;

    // Side

    from = ++to; // skip ','
    o.side = msg[from];
    if (unlikely(o.side != 'B' && o.side != 'S'))
    {
        m_stats.m_corruptedMsgNum++;                    
        return false; 
    }
    ++to;

    //Qty

    from = ++to;
    for (; msg[to] != ','; to++)
    {
        if(unlikely(msg[to] == '\0'))
        {
            m_stats.m_corruptedMsgNum++;        
            return false;
        }
    } 

    msg[to] = '\0';
    o.qty = strToPositiveUnsInt(msg+from);
    if (unlikely(o.qty == 0)) 
        return false;

    //Price

    from = ++to; // skip ','
    for (; msg[to] != '\0' && 
           msg[to] != '\n'; to++)
    {
    } 

    msg[to] = '\0';
    o.price = strToPositiveDouble(msg+from);
    if (unlikely(o.price == INVALID_PRICE)) 
        return false; 

    // hurray!
    END_LATENCY_TRACE("Parse Order") 
    return true;
}

/*************************************************************
 */
bool TextFileFeedParser::doParseTrade(char* msg, Trade& t)
{
    BEGIN_LATENCY_TRACE("Parse Trade") 
    size_t from = 0, to = 0;

    //Qty

    from = ++to; // skip ','
    for (; msg[to] != ','; to++)
    {
        if(unlikely(msg[to] == '\0'))
        {
            m_stats.m_corruptedMsgNum++;        
            return false;
        }
    } 

    msg[to] = '\0';
    t.qty = strToPositiveUnsInt(msg+from);
    if (unlikely(t.qty == 0)) 
        return false;

    //Price

    from = ++to; // skip ','
    for (; msg[to] != '\0'; to++)
    {
    } 

    msg[to] = '\0';
    t.price = strToPositiveDouble(msg+from);
    if (unlikely(t.price == INVALID_PRICE)) 
        return false; 
    
    END_LATENCY_TRACE("Parse Trade")
    return true;
}

/*************************************************************
 * Worth trying SIMD instruction for vector multiplication and
   addition to speed up string to integer conversion
 */
uint32_t TextFileFeedParser::strToPositiveUnsInt(const char* str)
{
    unsigned long int val = strtoul(str, 0, 10);    
    if (val == 0) 
    {
        m_stats.m_corruptedMsgNum++;        
    }    
    else if (val >= ULONG_MAX || 
             val >= UINT_MAX)/*since it has to be integer which*/
                             /*is equal to even on 64-bit platform*/ 
    {
        m_stats.m_invalidDataNum++;
        val = 0;
    }

    return val;
}

/*************************************************************
 * Worth trying SIMD instruction for vector multiplication and
   addition to speed up string to double conversion
 */
double TextFileFeedParser::strToPositiveDouble(const char* str)
{
    double val = strtod(str, 0);

    if (val == 0.0)
    {
        m_stats.m_corruptedMsgNum++;                            
    }
    else if (val == std::numeric_limits<double>::quiet_NaN() ||
             val <= -1 * HUGE_VAL ||
             val >= HUGE_VAL ||
             val < 0)
    {
        m_stats.m_invalidDataNum++;
        val = INVALID_PRICE;
    }

    return val;
}

