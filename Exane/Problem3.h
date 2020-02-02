#pragma once

#include <cstdint>
#include <map>
#include <assert.h>

namespace prob3
{
    struct Input
    {
         unsigned long days;
    };

    class Solution
    {
    public:
        struct PayRecord
        {
            unsigned long dailyRate;
            unsigned long totalGold;
        };

        unsigned long _maxDays;
        std::map<unsigned long, PayRecord> _payMap;
        explicit Solution(unsigned long maxDays)
            :_maxDays(maxDays)
        {
            unsigned long days = 0;            
            unsigned long money = 0;           
            unsigned long step = 1;

            while (days < _maxDays)
            {
                money += step * step;
                days += step;                
                _payMap.emplace(days, PayRecord{ step, money });
                ++step;
            }
        }
        auto process(const Input& data)
        {
            auto it = _payMap.upper_bound(data.days);
            if (it == std::end(_payMap))
                throw std::runtime_error
                ("invalid days input!");

            auto daysToDiscount = it->first - data.days;
            auto discount = daysToDiscount *
                 it->second.dailyRate;

            return std::make_pair(data.days,
                it->second.totalGold - 
                discount);
        }
    };

    class IOProcessor
    {
    public:
        template <class InputStream, class OutputStream>
            void go(InputStream& in,
                    OutputStream& out, 
                    unsigned long maxDays)
        {
            Solution s(maxDays);
            std::string line;
            while (std::getline(in, line))
            {
                auto num = toUint(line);
                if (0 == num) return;
                auto p = s.process({ num });
                out << p.first << " " 
                    << p.second
                    << "\n";
            }
        }
    };

    namespace test
    {
        template<unsigned long MaxDays>
        static void go()
        {
            {
                Solution s(MaxDays);
                assert(s.process({ 10 }).second == 30);
                assert(s.process({ 1 }).second == 1);
                assert(s.process({ 6 }).second == 14);
                assert(s.process({ 7 }).second == 18);
                assert(s.process({ 21 }).second == 91);
                assert(s.process({ 100 }).second == 945);
                assert(s.process({ 10000 }).second == 942820);
                assert(s.process({ 1000 }).second == 29820);
                assert(s.process({ 15 }).second == 55);
            }

            std::cout << "\n UnitTests for Problem 3 passed succesfully!";
        }
    }
}
