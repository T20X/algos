#ifndef _TRADE_HPP_
#define _TRADE_HPP_

#include <vector>
#include "Common.hpp"

struct Trade 
{
    enum BaseFields
    {
        QTY,
        PRICE,        
        FIELD_NUM
    };

    enum Side
    {
        BID = 'B',
        ASK = 'S'        
    };

    uint32_t qty;
    double price;    

    bool operator==(const Trade& another)
    {
        return (qty == another.qty && isPriceEqual(price, another.price));
    }
};

typedef std::vector<Trade> TradeList;

#endif //_TRADE_HPP_
