#pragma once

#include <string>
#include <sstream>

struct OrderBook
{
    int volume;
    double price;
    char side;
    string instrument;

    string str()
    {
        ostringstream o;
        o << "\n instrument: " << instrument << "; "
          << side << "; " << price << "@" << volume;
        return o.str();
    }
};
