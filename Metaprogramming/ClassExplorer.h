#pragma once

#include "../common/basics.h"
#include <string>
#include <boost/type_index.hpp>

namespace explorer
{
    using namespace boost::typeindex;    

    void quickTest()
    {
        struct ExploreMe
        {


            int x;
            int y;
            double z;
            std::string s;
        };

        int ExploreMe::*pX = &ExploreMe::x;
        double ExploreMe::*pZ = &ExploreMe::z;
        basics::print(basics::typeName<decltype(pX)>());
    }


}