#pragma once

#include <cstdint>

namespace prob4
{
    struct Input
    {
        struct Item
        {
            std::uint32_t price;
            std::uint32_t weight;
        };

        std::vector<Item> _items;
        std::vector<std::uint32_t> _family;
    };

    class Solution
    {
    public:
        auto process(const Input& d)
        {
            std::uint32_t totalValue = 0;
            auto itemsCount = d._items.size();
            for (auto personCapacity : d._family)
            {                
                std::vector<std::vector<std::uint32_t>> m(
                    itemsCount,
                    std::vector<std::uint32_t>(
                        personCapacity+(std::uint32_t)1U, 
                        0));

                for (std::uint32_t i = 0; 
                     i < itemsCount;
                     ++i)
                {
                    for (std::uint32_t w = 1;
                         w <= personCapacity; 
                         ++w)
                    {                                                   
                        auto itemWeight = d._items[i].weight;
                        auto itemPrice = d._items[i].price;

                        if (i == 0)
                        {
                            if(w >= itemWeight)
                              m[i][w] = itemPrice;
                        }
                        else if (w < itemWeight )
                            m[i][w] = m[i-1][w];
                        else
                        {
                            m[i][w] = std::max(itemPrice + m[i - 1][w - itemWeight],
                                m[i - 1][w]);
                        }
                    }
                }

                totalValue += m[itemsCount-1]
                               [personCapacity];
            }

            return totalValue;
        }
    };

    class IOProcessor
    {
    public:
        template <class InputStream, class OutputStream>
        void go(InputStream& in, OutputStream& out)
        {
            Solution s;
            std::string line;            
            std::getline(in, line);
            auto N = toUint(line);           
            std::uint32_t count = 0;
            while (count < N)
            {
                Input data;
                {
                    std::getline(in, line);
                    auto itemsNum = toUint(line);
                    for (std::uint32_t i = 0; i < itemsNum; ++i)
                    {
                        std::getline(in, line);
                        std::vector<std::string> itemStr;
                        {
                            split(line, itemStr, ' ');
                            if (itemStr.size() != 2)
                                throw std::runtime_error(
                                    "item must be represented as price and"
                                    "weight separated by space");

                            data._items.emplace_back(
                                Input::Item{ toUint(itemStr[0]),
                                             toUint(itemStr[1]) });
                        }
                    }

                    std::getline(in, line);
                    auto personNum = toUint(line);
                    for (std::uint32_t i = 0; i < personNum; ++i)
                    {
                        std::getline(in, line);
                        data._family.emplace_back(toUint(line));                        
                    }
                }

                auto r = s.process(data);
                out << r << "\n";
                ++count;
            }
        }
    };

    namespace test
    {
        static void go()
        {
            {
                Solution s;
                Input d;

                {
                    d._items = { {72,17} };
                    d._family = { 26 };
                }
                assert(72 == s.process(d));

                {
                    d._items = { {72,17} };
                    d._family = { };
                }
                assert(0 == s.process(d));


                {
                    d._items = { {} };
                    d._family = {1 };
                }
                assert(0 == s.process(d));

                {
                    d._items = { {72,17},{44,23},{31,24} };
                    d._family = { 26 };
                }
                assert(72 == s.process(d));

                {
                    d._items = { {72,17},{44,23},{31,24} };
                    d._family = { 26,16,24 };
                }
                assert(144 == s.process(d));

                {
                    d._items = { {72,17},{44,23},{1,10},{31,24} };
                    d._family = { 26,16,24};
                }
                assert(145 == s.process(d));

                {
                    d._items = { {10,10},{1,1} };
                    d._family = { 1,1,1 };
                }
                assert(3 == s.process(d));
            }

            std::cout << "\n UnitTests for Problem 4 passed succesfully!";

        }
    }
}

