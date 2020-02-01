#pragma once

#include <cstdint>
#include <map>
#include <list>
#include <assert.h>
#include <vector>
#include <algorithm>
#include <random>
#include <iostream>
#include <string>
#include "common.h"

namespace prob1
{
    struct Input
    {
        unsigned long num;
    };

    class Solution
    {
    public:
        using Value = decltype(Input::num);
        using ListIt = typename std::list<Value>::const_iterator;

        std::multimap<Value, ListIt> _numsOrdered;
        std::list<Value> _nums;
        ListIt _medianIt;

        Solution():_medianIt(std::end(_nums))
        {
        }
        
        Value process(const Input& data)
        {
            auto it = _numsOrdered.upper_bound(data.num);
            if (it != std::end(_numsOrdered))
            {
                auto numIt = _nums.emplace(it->second, data.num);
                _numsOrdered.emplace_hint(it, data.num, numIt);
            }
            else 
            {
                //othewise it is the very first or highest number
                auto numIt = _nums.emplace(std::end(_nums), data.num);
                _numsOrdered.emplace(data.num, numIt);
            }

            bool odd = isOdd(_nums.size());
            if (_medianIt == std::end(_nums))
                _medianIt = std::begin(_nums);
            else if (data.num >= *_medianIt)
            {
                if (odd)
                    _medianIt = std::next(_medianIt);
            }
            else
            {
                 if (!odd)
                     _medianIt = std::prev(_medianIt);
            }

            return odd ? 
                *_medianIt : 
                (*_medianIt + *std::next(_medianIt)) 
                  / 2;
        }
    };

    
    class IOProcessor
    {
    public:
        template <class InputStream, 
                  class OutputStream>
        void go(InputStream& in,
                OutputStream& out)
        {
            Solution s;
            std::string line;
            while (std::getline(in, line))           
                out << s.process({ toUint(line) })
                    << "\n";            
        }
    };

    namespace test
    {
        template <class Container>
        auto processAndCollectLast(Solution& s, Container&& c)
        {
            Solution::Value last{};//set to 0!
            for (auto v : std::forward<Container>(c))
                last = s.process({ v });
            return last;
        }

        template <class Container>
        auto median(Container&& c)
        {
            auto sz = std::forward<Container>(c).size();
            std::nth_element(std::forward<Container>(c).begin(),
                std::forward<Container>(c).begin() + sz / 2,
                std::forward<Container>(c).end());
            auto v1 = std::forward<Container>(c)[sz / 2];
            if (isOdd(sz))
                return v1;
            else
            {
                std::nth_element(std::forward<Container>(c).begin(),
                    std::forward<Container>(c).begin() + sz / 2 - 1,
                    std::forward<Container>(c).end());
                auto v2 = std::forward<Container>(c)[sz / 2 - 1];
                return (v1 + v2) / 2;
            }
        }


        template <size_t N, size_t SequenceLen>
        static void go()
        {
            {
                Solution s;
                assert(s.process({ 0 }) == 0);
            }

            {
                Solution s;
                std::vector<unsigned long> v{ 1, 3, 6, 2, 7, 8 };
                assert(test::processAndCollectLast(s, v) == test::median(v));
            }

            {
                Solution s;
                std::vector<unsigned long> v{ 1, 3, 6, 2, 7, 8, 9 };
                assert(test::processAndCollectLast(s, v) == test::median(v));
            }

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<unsigned long> d(
                0, std::numeric_limits<unsigned long>::max());

            for (size_t i = 0; i < N; ++i)
            {
                Solution s;
                std::vector<unsigned long> v;
                v.reserve(SequenceLen);
                for (size_t i = 0; i < SequenceLen; ++i)
                    v.emplace_back(d(gen));
                assert(test::processAndCollectLast(s, v) ==
                    test::median(v));
            }

            std::cout << "\n UnitTests for Problem 1 passed succesfully!";
        }
    }
}

