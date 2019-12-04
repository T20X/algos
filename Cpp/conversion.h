#pragma once

namespace conv
{
    struct A
    {
        operator int() { return 1;  }
    };
    int f(int lk) { std::cout << "test"; return 0; }

    static void test()
    {
        f(A());
    }
}

