#pragma once

#include <iterator>
#include <vector>
#include <iostream>
#include <list>
#include <forward_list>

namespace sorting
{   
    template <typename It, typename Comp = std::less<>>
    void insertionSort(It first, It last)
    {
        if (first == last)
            return;

        details::_insertionSort<It, Comp>(first, last,
         typename std::iterator_traits<It>::iterator_category());
    }

    namespace details
    {
        template <typename It, typename Comp>
        void _insertionSort(It first, It last, std::random_access_iterator_tag tag)
        {
            It limit = first;
            while (++limit != last)
            {  
                It  current = limit;   
                std::iterator_traits<It>::value_type v = *limit;
                while (current > first && Comp()(v, *--current))                               
                    iter_swap(current, std::next(current));                
            }
        }

        template <typename It, typename Comp>
        void _insertionSort(It first, It last, std::bidirectional_iterator_tag tag)
        {
            It limit = first;
            while (++limit != last)
            {
                It  current = limit;
                std::iterator_traits<It>::value_type v = *limit;
                while (current != first && Comp()(v, *--current))
                    iter_swap(current, std::next(current));
            }
        }
    }

    void testInsertionSort()
    {
        std::vector<int> v{ 8, 1, 6, 9, 12, 0, 1, 5 };
        insertionSort(v.begin(), v.end());
        std::copy(v.begin(), v.end(), std::ostream_iterator<int>(std::cout, "|"));
        std::cout << std::endl;

        {
            std::list<int> l{ 8, 1, 6, 9, 12, 0, 1, 5 };
            insertionSort(l.begin(), l.end());
            std::copy(l.begin(), l.end(), std::ostream_iterator<int>(std::cout, "|"));
            std::cout << std::endl;
        }
    }
}