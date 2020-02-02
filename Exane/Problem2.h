#pragma once


#include <cstdint>
#include <map>
#include <list>
#include <unordered_set>
#include <vector>

namespace prob2
{
    struct Input
    {
        struct Coordinate
        {
            int x;
            int y;

            bool operator==(const Coordinate& l) const
            {
                return l.x == x && l.y == y;
            }
        };

        struct Hash
        {            
            size_t operator()(const Coordinate& c) const
            {
                return (c.x * 0x1f1f1f1f) ^ c.y;
            }
        };

        bool contains(const Coordinate& c) const
        {
            return _coordinates.find(c) != std::end(_coordinates);
        }

        std::unordered_set<Coordinate,Hash> _coordinates;
    };

    class Solution
    {
    public:
        auto process(const Input& d)
        {
            int result = 0;
            for (const auto& point : d._coordinates)
                for (const auto& pointAbove : d._coordinates)               
                    if (pointAbove.y > point.y)
                    {
                        if (point.x - pointAbove.x == pointAbove.y - point.y)
                        {
                            if (d.contains({ point.x, pointAbove.y }) &&
                                d.contains({ pointAbove.x, point.y }))                               
                                 ++result;
                        }
                        else if (point.x == pointAbove.x) //diagonal case!
                        {
                            auto halfSide = (pointAbove.y - point.y) / 2;
                            if (halfSide > 0)
                            {
                                if (d.contains({ point.x + halfSide, pointAbove.y - halfSide }) &&
                                    d.contains({ point.x - halfSide, pointAbove.y - halfSide }))                                   
                                      ++result;
                            }
                        }

                    }

            return result;
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
            while (std::getline(in, line))
            {
                auto itemsNum = toInt(line);
                if (0 == itemsNum)
                    return;

                Input data;
                {                    
                    for (int i = 0; i < itemsNum; ++i)
                    {
                        std::getline(in, line);
                        std::vector<std::string> itemStr;
                        {
                            split(line, itemStr, ' ');
                            if (itemStr.size() != 2)
                                throw std::runtime_error(
                                    "item must be represented as x and"
                                    "y separated by space");

                            data._coordinates.emplace(Input::Coordinate {
                                toInt(itemStr[0]),
                                toInt(itemStr[1]) }
                            );
                        }
                    }
                }

                out << s.process(data) << "\n";
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
                d._coordinates = { {1,0},{0,1},{1,1},{0,0} };
                assert(1 == s.process(d));

                d._coordinates = { {0,0},{1,0},{2,0},{0,2},{1,2},{2,2},{0,1},{1,1},{2,1} };
                assert(6 == s.process(d));

                d._coordinates = { {0,0}, {0,0 } };
                assert(0 == s.process(d));

                d._coordinates = { {-2,5}, {3,7 },{0,0},{5,2} };
                assert(0 == s.process(d));

                d._coordinates = { {0,0},{-1,-1},{0,-1},{-1,0} };
                assert(1 == s.process(d));
            }

            std::cout << "\n UnitTests for Problem 2 passed succesfully!";
        }
    }
}

