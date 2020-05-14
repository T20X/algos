#pragma once

namespace init
{
    struct A
    {
        int i;
    };

    struct B : A
    {
       // using A::A;

    };
    static void test()
    {
        B b1  { 1 };
        //{}
    }
}
