#ifndef _SUMMARY_HPP_
#define _SUMMARY_HPP_

#include <ostream>

struct Summary
{
public:
    int m_corruptedMsgNum;
    int m_dupAddOrderIdNum;
    int m_tradeWithNoOrderNum;
    int m_delWithNoOrderNum;
    int m_matchButNoTradeNum;
    int m_invalidDataNum;

    void dump(std::ostream& o) const
    {
        o << "\n\n\n";

        o << "***********************************************"  << "\n";
        o << "                SUMMARY                        "  << "\n";
        o << "***********************************************"  << "\n";

        o << "Corrupted messages : "  
          << m_corruptedMsgNum << "\n";

        o << "Duplicated order ids (duplicate adds) : " 
          << m_dupAddOrderIdNum << "\n";

        o << "Trades with no corresponding order : " 
          << m_tradeWithNoOrderNum << "\n";

        o << "Removes with no corresponding order : " 
          << m_delWithNoOrderNum << "\n";

        o << "Best sell order price at or below best "
          << "buy order price, but no trades occur : " 
          << m_matchButNoTradeNum << "\n";

        o << "Negative, missing, or out-of-bounds prices, "
             "quantities, order ids : " 
             << m_invalidDataNum;

        o << "\n\n\n" << std::endl;
    }
};

#endif // _SUMMARY_HPP_

