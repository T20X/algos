#pragma once


#include <cstdint>
#include <map>
#include <list>
#include <unordered_map>
#include <vector>

namespace prob2
{
    struct Input
    {
        struct Coordinate
        {
            int x;
            int y;
        };

        std::vector<Coordinate> _coordinates;
    };

    class Solution
    {
    public:
        auto process(const Input& d)
        {
            std::unordered_map<
                std::pair<int, int>, 
                std::vector<int>,
                IntegralPairHash>
                _points;

            for (const auto& point : d._coordinates)            
                for (const auto& pointAbove : d._coordinates)                
                    if (point.x == pointAbove.x && pointAbove.y > point.y)                    
                        _points[std::make_pair(point.y, pointAbove.y)]
                          .emplace_back(point.x);
                
            int result = 0;
            for (auto& record : _points)
            {
                auto y = record.first.first;
                auto yAbove = record.first.second;
                auto target = yAbove - y;

                sort(std::begin(record.second), 
                     std::end(record.second));

                size_t l = 0, r = 0;                
                while (r < record.second.size())
                {
                    auto diff = record.second[r] -
                                record.second[l];
                    if (diff == target)
                    {
                        ++result;
                        ++l;
                        ++r;
                    }
                    else if (diff < target)
                        ++r;
                    else
                        ++l;
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

                            data._coordinates.emplace_back(
                                Input::Coordinate{ toInt(itemStr[0]),
                                                   toInt(itemStr[1]) });
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
                assert(5 == s.process(d));

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

