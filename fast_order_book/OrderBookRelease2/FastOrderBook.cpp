#include <iostream>
#include <bitset>

#include "FastOrderBook.hpp"
#include "FastLogger.hpp"

/*************************************************************
 */
FastOrderBook::FastOrderBook(
    double priceRange, 
    double tickSize)
     : m_tickSize(tickSize),
       m_topMid(INVALID_PRICE),
       m_pivotPos(INVALID_POS),
       m_maxTicksNum(0),
       m_topBid(INVALID_PRICE),
       m_topBidPos(INVALID_POS),
       m_topAsk(INVALID_PRICE),
       m_topAskPos(INVALID_POS),
       m_curentTradePos(0/*intentionally not INVALID_POS*/),
       m_writeTradePos(0/*intentionally not INVALID_POS*/)
{
    if (tickSize < PRICE_EPSILON)
    {
        throw std::runtime_error("tickSize is less then PRICE_EPSILON");
    }

    m_maxTicksNum = priceRange / tickSize;
    m_maxTicksNum -= m_maxTicksNum % uint64BitsNum;
    m_maxTicksNum += uint64BitsNum;
    m_maxTicksNum *= 2;

    m_pivotPos = m_maxTicksNum / 2 - 1;

    m_bids.prices.resize(m_maxTicksNum);
    m_bids.priceLevelBitMask.resize(m_maxTicksNum / uint64BitsNum, 0); 
    m_bids.bestPriceLimitPos = m_maxTicksNum - 1;
    m_bids.worsePriceLimitPos = 0;
    m_bids.bestBitMaskPos = (m_bids.bestPriceLimitPos + 1) / uint64BitsNum;
    m_bids.worseBitMaskPos = (m_bids.worsePriceLimitPos + 1) / uint64BitsNum;

    m_asks.prices.resize(m_maxTicksNum);
    m_asks.priceLevelBitMask.resize(m_maxTicksNum / uint64BitsNum, 0); 
    m_asks.bestPriceLimitPos = 0;
    m_asks.worsePriceLimitPos = m_maxTicksNum - 1;
    m_asks.bestBitMaskPos = (m_asks.bestPriceLimitPos + 1) / uint64BitsNum;
    m_asks.worseBitMaskPos = (m_asks.worsePriceLimitPos + 1) / uint64BitsNum;

    m_odersMapping.resize(ORDER_BOOK_ORDER_MAPPING_SIZE);

    m_clearMaskForMostBits.resize(uint64BitsNum+1, 0);
    for (int i = uint64BitsNum-1; i >= 0; i--)
    {
        m_clearMaskForMostBits[i] = m_clearMaskForMostBits[i+1] << 1;
        m_clearMaskForMostBits[i]++; 
    }

    m_clearMaskForLeastBits.resize(uint64BitsNum+1, 0);
    for (int i = uint64BitsNum-1; i >= 0; i--)
    {
        m_clearMaskForLeastBits[i] = m_clearMaskForMostBits[i] << i;
    }
    
    m_tradeQueue.resize(ORDER_BOOK_TRADE_Q_SIZE);
}

/*************************************************************
 */
FastOrderBook::~FastOrderBook()
{
}

/*************************************************************
 */
void FastOrderBook::warmup()
{
    BookOrder toWarmUpMemoryAllocator = {1,1};

    m_bids.prices[0].orders.push_back(toWarmUpMemoryAllocator);
    m_bids.prices[0].orders.pop_back();
}

/*************************************************************
 */
bool FastOrderBook::insertOrder(const Order& newBorn)
{
    BEGIN_LATENCY_TRACE("Book Insert")
    bool isDuplicated = false;

     if(newBorn.side == Order::ASK)
         isDuplicated = doInsertAskOrder(newBorn);
     else if(newBorn.side == Order::BID)
         isDuplicated = doInsertBidOrder(newBorn);

    END_LATENCY_TRACE("Book Insert")
    return isDuplicated;
}

/*************************************************************
 */
struct BidPriceLimitCond
{
    bool operator()(double limit, double current)
    {
        return isLeftLess(current, limit) ? true : false;
    }
};

/*************************************************************
 */
struct AskPriceLimitCond
{
    bool operator()(double limit, double current)
    {
        return isLeftGreater(current, limit) ? true : false;
    }
};

/*************************************************************
 */
bool FastOrderBook::doInsertBidOrder(const Order& newBorn)
{
    if (unlikely(m_bids.bestPriceLimit == INVALID_PRICE))
    {
        //First order in the book, continue book boostrapping
        double spread = (m_maxTicksNum / 2) * m_tickSize;
        m_bids.bestPriceLimit = newBorn.price + spread;
        m_bids.worsePriceLimit = newBorn.price - spread;
    }

    if (unlikely(isLeftGreater(newBorn.price, m_bids.bestPriceLimit)))
        throw std::runtime_error("Bid order price hits the top limit!");

    if (unlikely(isLeftLess(newBorn.price, m_bids.worsePriceLimit)))
        throw std::runtime_error("Bid order price hits the bottom limit!");
    
    size_t pricePos = m_bids.worsePriceLimitPos + (newBorn.price - m_bids.worsePriceLimit) / m_tickSize; 
    PriceLevel& level = m_bids.prices[pricePos];

    BookOrder order = {newBorn.id, newBorn.qty};
    level.orders.push_back(order);

    if(unlikely(!m_odersMapping.insert(std::make_pair(
        newBorn.id, --level.orders.end())).second))
    {
        //duplicated order
        level.orders.pop_back();
        return true;
    }

    if (!level.occupied)
    {
        level.occupied = true;

        level.prev = findPrevBidNode(pricePos);
        if (likely(level.prev != INVALID_POS))
        {
            PriceLevel& prevLevel = m_bids.prices[level.prev];
            prevLevel.next = pricePos;
        }

        level.next = findNextBidNode(pricePos);
        if (likely(level.next != INVALID_POS))
        {
            PriceLevel& nextLevel = m_bids.prices[level.next];
            nextLevel.prev = pricePos;
        }

        level.price = newBorn.price;
        setBitTo1(pricePos, &m_bids.priceLevelBitMask[0]);

    }
    
    level.totalQty += order.qty;
    if (unlikely(m_topBidPos < pricePos || m_topBidPos == INVALID_POS))
    {
        if (likely(m_topBidPos != INVALID_POS))
        {
            PriceLevel& prevKing = m_bids.prices[m_topBidPos];
            prevKing.prev = pricePos;
        }

        m_topBid = level.price;
        m_topBidPos = pricePos;

        if (likely(isCrossed())) 
        {
            BEGIN_LATENCY_TRACE("Anticipate Trades") 
            anticipateTrades(AskPriceLimitCond(),
                             m_asks,
                             m_topAskPos,
                             order.qty,
                             level.price); 
            END_LATENCY_TRACE("Anticipate Trades")
        }
    }

    calcTop();
    return false;
}

/*************************************************************
 */
size_t FastOrderBook::findPrevBidNode(size_t pos)
{
    size_t ret = INVALID_POS;

    uint64_t bits = 0; 
    size_t bytePos = ++pos / uint64BitsNum;
    size_t shift = pos % uint64BitsNum;

    while (bytePos < m_bids.bestBitMaskPos)
    {
        bits = m_bids.priceLevelBitMask[bytePos];

        if (likely(shift)) 
        {
            bits &= m_clearMaskForLeastBits[shift];
            shift = 0;
        }

        if (likely(bits)) 
        {
            ret = static_cast<size_t>(findLeast1Bit(bits)) + 
                  bytePos * uint64BitsNum;
            break;
        }

        bytePos++;
    }

    return ret;
}

/*************************************************************
 */
size_t FastOrderBook::findNextBidNode(size_t pos)
{
    size_t ret = INVALID_POS;

    uint64_t bits = 0; 
    int bytePos = pos / uint64BitsNum; //bytePos must be int :)
    size_t shift =  uint64BitsNum - pos % uint64BitsNum;

    while (bytePos >= m_bids.worseBitMaskPos)
    {
        bits = m_bids.priceLevelBitMask[bytePos];

        if (likely(shift)) 
        {
            bits &= m_clearMaskForMostBits[shift];
            shift = 0;
        }

        if (likely(bits)) 
        {
            ret = static_cast<size_t>(findMost1Bit(bits)) + 
                  bytePos * uint64BitsNum;        
            break;
        }

        if (unlikely(--bytePos < 0)) break;
    }
    
    return ret;
}

/*************************************************************
 */
bool FastOrderBook::doInsertAskOrder(const Order& newBorn)
{
    if (unlikely( m_asks.bestPriceLimit == INVALID_PRICE))
    {
        //First order in the book, continue book boostrapping
        double spread = (m_maxTicksNum / 2) * m_tickSize;
        m_asks.bestPriceLimit = newBorn.price - spread;
        m_asks.worsePriceLimit = newBorn.price + spread;
    }

    if (unlikely(isLeftLess(newBorn.price, m_asks.bestPriceLimit)))
        throw std::runtime_error("Ask order price hits the top limit!");

    if (unlikely(isLeftGreater(newBorn.price, m_asks.worsePriceLimit)))
        throw std::runtime_error("Ask order price hits the bottom limit!");
    
    size_t pricePos = m_asks.bestPriceLimitPos + (newBorn.price - m_asks.bestPriceLimit) / m_tickSize; 
    PriceLevel& level = m_asks.prices[pricePos];

    BookOrder order = {newBorn.id, newBorn.qty};
    level.orders.push_back(order);

    if(unlikely(!m_odersMapping.insert(std::make_pair(
        newBorn.id, --level.orders.end())).second))
    {
        //duplicated order
        level.orders.pop_back();
        return true;
    }

    if (!level.occupied)
    {
        level.occupied = true;

        level.prev = findPrevAskNode(pricePos);
        if (likely(level.prev != INVALID_POS))
        {
            PriceLevel& prevLevel = m_asks.prices[level.prev];
            prevLevel.next = pricePos;
        }

        level.next = findNextAskNode(pricePos);
        if (likely(level.next != INVALID_POS))
        {
            PriceLevel& nextLevel = m_asks.prices[level.next];
            nextLevel.prev = pricePos;
        }

        level.price = newBorn.price; 
        setBitTo1(pricePos, &m_asks.priceLevelBitMask[0]);

    }
    
    level.totalQty += order.qty;
    if (unlikely(m_topAskPos > pricePos || m_topAskPos == INVALID_POS))
    {
        if (likely(m_topAskPos != INVALID_POS))
        {
            PriceLevel& prevKing = m_asks.prices[m_topAskPos];
            prevKing.prev = pricePos;
        }

        m_topAsk = level.price;
        m_topAskPos = pricePos;

        if (likely(isCrossed())) 
        {
            BEGIN_LATENCY_TRACE("Anticipate Trades") 
            anticipateTrades(BidPriceLimitCond(),
                             m_bids,
                             m_topBidPos,
                             order.qty,
                             level.price); 
            END_LATENCY_TRACE("Anticipate Trades")
        }
    }

    calcTop();
    return false;
}

/*************************************************************
 */
size_t FastOrderBook::findNextAskNode(size_t pos)
{
    size_t ret = INVALID_POS;

    uint64_t bits = 0; 
    size_t bytePos = ++pos / uint64BitsNum;
    size_t shift = pos % uint64BitsNum;

    while (bytePos < m_asks.worseBitMaskPos)
    {
        bits = m_asks.priceLevelBitMask[bytePos];

        if (likely(shift)) 
        {
            bits &= m_clearMaskForLeastBits[shift];
            shift = 0;
        }

        if (likely(bits)) 
        {
            ret = static_cast<size_t>(findLeast1Bit(bits)) + 
                  bytePos * uint64BitsNum;
            break;
        }

        bytePos++;
    }

    return ret;
}

/*************************************************************
 */
size_t FastOrderBook::findPrevAskNode(size_t pos)
{
    size_t ret = INVALID_POS;

    uint64_t bits = 0; 
    int bytePos = pos / uint64BitsNum; // bytePos must be int :)
    size_t shift =  uint64BitsNum - pos % uint64BitsNum;

    while (bytePos >= m_asks.bestBitMaskPos)
    {
        bits = m_asks.priceLevelBitMask[bytePos];

        if (likely(shift)) 
        {
            bits &= m_clearMaskForMostBits[shift];
            shift = 0;
        }

        if (likely(bits)) 
        {
            ret = static_cast<size_t>(findMost1Bit(bits)) + 
                  bytePos * uint64BitsNum;        
            break;
        }

        if (unlikely(--bytePos < 0)) break;
    }
    
    return ret;
}

/*************************************************************
 */
void FastOrderBook::modifyOrder(const Order& modified)
{
    BEGIN_LATENCY_TRACE("Book Modify") 

    if(modified.side == Order::BID) 
        doModifyBidOrder(modified);
    else if(modified.side == Order::ASK) 
        doModifyAskOrder(modified);

    END_LATENCY_TRACE("Book Modify") 
}

/*************************************************************
 */
void FastOrderBook::doModifyBidOrder(const Order& modified)
{
    OrderToBookOrderMap::iterator it = m_odersMapping.find(modified.id);
    if (likely(it != m_odersMapping.end()))
    {
        size_t pricePos = m_bids.worsePriceLimitPos + (modified.price - m_bids.worsePriceLimit) / m_tickSize; 
        PriceLevel& level = m_bids.prices[pricePos];

        uint32_t diff = it->second->qty - modified.qty;
        level.totalQty += -1 * diff;
        
        it->second->qty = modified.qty; 
    } 
}

/*************************************************************
 */
void FastOrderBook::doModifyAskOrder(const Order& modified)
{
    OrderToBookOrderMap::iterator it = m_odersMapping.find(modified.id);
    if (likely(it != m_odersMapping.end()))
    {
        size_t pricePos = m_asks.bestPriceLimitPos + (modified.price - m_asks.bestPriceLimit) / m_tickSize; 
        PriceLevel& level = m_asks.prices[pricePos];

        uint32_t diff = it->second->qty - modified.qty;
        level.totalQty += -1 * diff;

        it->second->qty = modified.qty;
    } 
}

/*************************************************************
 */
bool FastOrderBook::deleteOrder(const Order& old)
{
    BEGIN_LATENCY_TRACE("Book Delete")
    bool isDuplicated = false;

    if(old.side == Order::BID) 
        isDuplicated = doDeleteBidOrder(old);
    else if(old.side == Order::ASK) 
        isDuplicated = doDeleteAskOrder(old);

    END_LATENCY_TRACE("Book Delete") 
    return isDuplicated;
}

/*************************************************************
 */
bool FastOrderBook::doDeleteBidOrder(const Order& old)
{
    OrderToBookOrderMap::iterator it = m_odersMapping.find(old.id);    
    bool notFound = (it == m_odersMapping.end());
    if (likely(!notFound))
    {
        size_t pricePos = m_bids.worsePriceLimitPos + (old.price - m_bids.worsePriceLimit) / m_tickSize; 
        PriceLevel& level = m_bids.prices[pricePos];

        level.orders.erase(it->second);
        m_odersMapping.erase(it);

        level.totalQty -= old.qty;
        if (!level.totalQty)
        {
            // free price level

            level.occupied = false;
            if (likely(level.prev != INVALID_POS))
            {
                PriceLevel& prevLevel = m_bids.prices[level.prev];
                prevLevel.next = level.next;
            }

            if (likely(level.next != INVALID_POS))
            {
                PriceLevel& nextLevel = m_bids.prices[level.next];
                nextLevel.prev = level.prev;
                if (unlikely(m_topBidPos == pricePos))
                {
                    m_topBidPos = level.next;
                    m_topBid = nextLevel.price;
                }
            }
            else  if (unlikely(m_topBidPos == pricePos))
            {
                m_topBidPos = INVALID_POS;
                m_topBid = INVALID_PRICE;
            }

            level.prev = INVALID_POS;
            level.next = INVALID_POS;

            setBitTo0(pricePos, &m_bids.priceLevelBitMask[0]);
        }

        calcTop(); 
    }        
    
    return notFound;
}

/*************************************************************
 */
bool FastOrderBook::doDeleteAskOrder(const Order& old)
{
    OrderToBookOrderMap::iterator it = m_odersMapping.find(old.id);    
    bool notFound = (it == m_odersMapping.end());
    if (likely(!notFound))
    {
        size_t pricePos = m_asks.bestPriceLimitPos + (old.price - m_asks.bestPriceLimit) / m_tickSize; 
        PriceLevel& level = m_asks.prices[pricePos];

        level.orders.erase(it->second);
        m_odersMapping.erase(it);

        level.totalQty -= old.qty;
        if (!level.totalQty) 
        {
            // free price level

            level.occupied = false;
            if (likely(level.prev != INVALID_POS))
            {
                PriceLevel& prevLevel = m_asks.prices[level.prev];
                prevLevel.next = level.next;
            }

            if (likely(level.next != INVALID_POS))
            {
                PriceLevel& nextLevel = m_asks.prices[level.next];
                nextLevel.prev = level.prev;
                if (unlikely(m_topAskPos == pricePos))
                {
                    m_topAskPos = level.next;
                    m_topAsk = nextLevel.price;
                }
            }
            else  if (unlikely(m_topAskPos == pricePos))
            {
                m_topAskPos = INVALID_POS;
                m_topAsk = INVALID_PRICE;
            }

            level.prev = INVALID_POS;
            level.next = INVALID_POS;

            setBitTo0(pricePos, &m_asks.priceLevelBitMask[0]);
        }

        calcTop();
        
    }        
    
    return notFound;
}

/*************************************************************
 */
void FastOrderBook::calcTop()
{
    if (unlikely(m_topBidPos == INVALID_POS || 
                 m_topAskPos == INVALID_POS))
    {
        m_topMid = INVALID_PRICE;
    }
    else
    {
        m_topMid = (m_topBid + m_topAsk) / 2.0;
    }
}

/*************************************************************
 */
double FastOrderBook::getTopMid() const 
{
    return m_topMid; 
}

/*************************************************************
 */
bool FastOrderBook::match(const Trade& trade)
{
    bool ours = false;
    if (unlikely(m_curentTradePos == m_writeTradePos))
         return ours;

    const std::pair<double, uint32_t>& anticipatedTrade =
        m_tradeQueue[m_curentTradePos];

    if (likely(isPriceEqual(anticipatedTrade.first, trade.price) && 
                anticipatedTrade.second == trade.qty))  
    {
        m_curentTradePos++;
        m_curentTradePos %= m_tradeQueue.size();
        ours = true; 
    }

    return ours;
}

/*************************************************************
 */
void FastOrderBook::dumpToHuman(FastLogger& o) const
{
    BEGIN_LATENCY_TRACE("Book Dump")

    o << "\n\n";

    size_t askPricePos = m_topAskPos;
    size_t bidPricePos = m_topBidPos;

    while(bidPricePos != INVALID_POS || askPricePos != INVALID_POS)
    {    
        FastLogger::PriceLevel toOutput = {
            INVALID_PRICE, 
            INVALID_PRICE, 
            0, 0}; 

        if (likely(bidPricePos != INVALID_POS))
        {            
            const PriceLevel& bidLevel = m_bids.prices[bidPricePos];
            toOutput.bidPrice = bidLevel.price;
            toOutput.bidQty = bidLevel.totalQty;
            bidPricePos = bidLevel.next;
        }        

        if (likely(askPricePos != INVALID_POS))
        {        
            const PriceLevel& askLevel = m_asks.prices[askPricePos];
            toOutput.askPrice = askLevel.price;
            toOutput.askQty = askLevel.totalQty;
            askPricePos = askLevel.next;
        }    

        o << toOutput;
    }

    o << "\n\n"; 

    END_LATENCY_TRACE("Book Dump")
}

/*************************************************************
 */
bool FastOrderBook::isCrossed() const
{
    return  (m_topBid > m_topAsk || isPriceEqual(m_topBid, m_topAsk)) &&
             m_topBidPos != INVALID_POS && 
             m_topAskPos != INVALID_POS;
}

/*************************************************************
 */
size_t FastOrderBook::howManyTradesExpected()
{
    if (likely(m_writeTradePos >= m_curentTradePos))
    {
        return m_writeTradePos - m_curentTradePos;
    }
    else
    {
        return m_tradeQueue.size() - m_curentTradePos +
               m_writeTradePos;
    }
}

/*************************************************************
 */
void FastOrderBook::clearExpectedTrade(size_t NfromStart)
{
    m_curentTradePos = (m_curentTradePos + NfromStart) % 
                        m_tradeQueue.size();
}

/*************************************************************
 */
void FastOrderBook::clear()
{
}

