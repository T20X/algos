#pragma once
#include <iostream>
#include <exception>
namespace base
{
    using namespace std;
    struct A
    {
        A()
        {
            //if constructor throws then destructor does not get called
            throw std::runtime_error("haha");
        }
        ~A()
        {
            cout << "\n A::~";
        }
    };

    static void test()
    {
        try
        {
            A a1;
        }
        catch (...)
        {
            cout << "\n ass";
        }

    }
}
