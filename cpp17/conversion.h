#pragma once
#include <string>
namespace conv
{
    struct A
    {
        string str;
    };
    const auto f()
    {
        const A a1;
        return a1;
    }
    static void test()
    {
        auto r = f();
        //decltype(auto) r = f();  - not going to work
        r.str = "sadfsdf";
    }
}

