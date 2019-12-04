#pragma once

#include <iostream>
#include "../common/basics.h"

namespace templates
{
    using namespace std;
    struct base
    {
        template <typename F>
        F f1()
        {
            basics::print("base::f1()");
            return {};
        }
    };


    struct derived : base
    {
    };

    template <typename T>
    auto f(T x1, T x2)
    {
        basics::print("f is tempalte");
        return x1 + x2;
    }

    int f(int, int)
    {
        basics::print("f non tempalte");
        return 10;
    }

    void test()
    {
        f(110, 10);
        derived d;
        d.f1<int>();
        d.f1<double>();
    }
}
