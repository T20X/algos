#ifndef _FAST_ORDER_BOOK_HPP_
#define _FAST_ORDER_BOOK_HPP_

#include <list>
#include <vector>
#include <ext/hash_map>
#include <iostream>

#include "STLCustomAllocator.hpp"
#include "Common.hpp"
#include "Config.hpp"
#include "FastLogger.hpp"
#include "Order.hpp"
#include "Trade.hpp"

/*********************************************************************************
 * It is basically based on sparse array. Each array pos represents a price level
 * rounded to the tick size. Price level itself contains a list of orders at that level.
 * Apart from it, a hash map is used to map order ids to order iterators from the order list at price level. 
 * Another important detail is a bit array to identify price levels which have a valid price;
 * bit pos maps to the corresponding price level pos.
 *  
 *
 * Next step to improve performance might be to allow multiple buckets which
 * would maintain mininum acceptable diffrence (multiplied by 2) between best and worse price.
 * If best price crossed bucket's limit then the less significant half of the bucket would be 
 * deallocated and any subsequent orders/updates to the cut half would be simply ignored.
 * Deallocation itself might be delegated to another thread to improve performance even further 
 */

class FastOrderBook
{
public:
    FastOrderBook(
        double priceRange, //actual price range will be multiplied by 2 
        double tickSize);

    ~FastOrderBook();

    void warmup();

    bool insertOrder(const Order& newBorn);    
    void modifyOrder(const Order& modified);
    bool deleteOrder(const Order& old);

    double getTopMid() const;

    bool isCrossed() const;
    bool match(const Trade& trade);
    size_t howManyTradesExpected();
    void clearExpectedTrade(size_t NfromStart);

    void dumpToHuman(FastLogger& o) const;
    void clear();

private:
    bool doInsertAskOrder(const Order& newBorn);
    void doModifyAskOrder(const Order& modified);
    bool doDeleteAskOrder(const Order& old);

    bool doInsertBidOrder(const Order& newBorn);
    void doModifyBidOrder(const Order& modified);
    bool doDeleteBidOrder(const Order& old);

    size_t findNextBidNode(size_t pos);
    size_t findPrevBidNode(size_t pos);

    size_t findNextAskNode(size_t pos);
    size_t findPrevAskNode(size_t pos);

    void calcTop();

    class BookBucket;
    template <typename LimitFunc>
    void anticipateTrades(
        LimitFunc limitCond,
        BookBucket& bucket,
        size_t topPos,
        uint32_t limitQty, 
        double limitPrice);

private:
    struct BookOrder
    {
        uint32_t id;
        uint32_t qty;
    };

    struct PriceLevel
    {
        PriceLevel()
            : occupied(false),
              prev(INVALID_POS),
              next(INVALID_POS),
              totalQty(0),
              price(INVALID_PRICE)
        {}
             
        bool occupied;
        size_t prev;
        size_t next;
        uint32_t totalQty;
        double price;
        
        typedef std::list<BookOrder, STLCustomAllocator<BookOrder> > BookOrderList;
        BookOrderList orders;
    };


    struct BookBucket
    {
        BookBucket()
            : bestPriceLimit(INVALID_PRICE),
              bestPriceLimitPos(INVALID_POS),
              bestBitMaskPos(INVALID_POS),
              worsePriceLimit(INVALID_PRICE),
              worsePriceLimitPos(INVALID_POS),
              worseBitMaskPos(INVALID_POS)
        {}

        double bestPriceLimit;
        size_t bestPriceLimitPos;
        size_t bestBitMaskPos;

        double worsePriceLimit;
        size_t worsePriceLimitPos;
        size_t worseBitMaskPos;

        std::vector<PriceLevel> prices; 

        //to identify taken/free price levels
        std::vector<uint64_t> priceLevelBitMask; 
    };

    FastOrderBook(const FastOrderBook& another);
    FastOrderBook& operator==(const FastOrderBook& another);

private:
    typedef __gnu_cxx::hash_map<uint32_t/*orderId*/, 
        PriceLevel::BookOrderList::iterator> OrderToBookOrderMap;

    double m_tickSize;
    double m_topMid;  

    size_t m_pivotPos;
    size_t m_maxTicksNum;

    double m_topBid;
    size_t m_topBidPos;
    BookBucket m_bids;

    double m_topAsk;
    size_t m_topAskPos;
    BookBucket m_asks;

    const static size_t uint64BitsNum = sizeof(uint64_t) * 8;

    std::vector<uint64_t> m_clearMaskForMostBits;
    std::vector<uint64_t> m_clearMaskForLeastBits;

    OrderToBookOrderMap m_odersMapping;


    typedef std::vector< std::pair<double/*price*/, 
                                   uint32_t/*qty*/> > TradeBook;
    size_t m_curentTradePos;
    size_t m_writeTradePos;
    TradeBook m_tradeQueue;
};

template <typename LimitFunc>
void FastOrderBook::anticipateTrades(
    LimitFunc limitCond,
    BookBucket& bucket,
    size_t topPos,
    uint32_t limitQty, 
    double limitPrice)
{
    uint32_t unfilledQty = limitQty;
    size_t bucketIterator = topPos;

    while (bucketIterator != INVALID_POS)
    {
        PriceLevel& level = bucket.prices[bucketIterator];
        if (limitCond(limitPrice, level.price)) return;

        PriceLevel::BookOrderList::iterator it = level.orders.begin(); 

        do
        {
            if (unfilledQty == 0) return; 

            uint32_t tradeQty = (it->qty > unfilledQty ? unfilledQty : it->qty); 
            m_tradeQueue[m_writeTradePos] = std::make_pair(level.price, tradeQty);
            unfilledQty -= tradeQty;

            m_writeTradePos++;
            m_writeTradePos %= m_tradeQueue.size();
            if (m_writeTradePos == m_curentTradePos)
            {
                size_t prevSize = m_tradeQueue.size();
                m_tradeQueue.resize(prevSize * 2);
                m_writeTradePos = prevSize;
            }
            
            it++;
        }
        while (it != level.orders.end());

        bucketIterator = level.next;
    }
}

#endif // _FAST_ORDER_BOOK_HPP_

