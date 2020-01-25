#ifndef _ORDER_HPP_
#define _ORDER_HPP_

#include "Common.hpp"

struct Order
{
    enum BaseFields
    {
        ID,
        SIDE,
        QTY,
        PRICE,
        FIELD_NUM
    };

    enum Side
    {
        BID = 'B',
        ASK = 'S'        
    };
    
    uint32_t id;    
    char side;    
    uint32_t qty;
    double price;        
};

struct OrderAdd : public Order
{};

struct OrderModify : public Order
{};

struct OrderDel : public Order
{};

#endif // _ORDER_HPP
