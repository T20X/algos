#ifndef _UNIFIED_MSG_HPP_
#define _UNIFIED_MSG_HPP_

#include "Common.hpp"
#include "Order.hpp"
#include "Trade.hpp"

struct UnifiedMsg
{
    enum MessageType
    {
        TRADE_MSG,
        ORDER_NEW_MSG,
        ORDER_MODIFY_MSG,
        ORDER_DELETE_MSG,
        MSG_NUM
    };

    char busy;
    char padding[CACHE_LINE_SIZE-sizeof(char)];

    MessageType type;
    uint64_t timestamp;

    union AllMsgUnion
    {
        struct OrderAdd    orderA;
        struct OrderDel    orderD;
        struct OrderModify orderM;
        struct Trade       trade;
    } agnosticMsg;

}; 

#endif // _UNIFIED_MSG_HPP_
