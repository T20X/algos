#pragma once
#include <iostream>
#include <exception>
#include "../common/basics.h"
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

    template <typename T>
    void f() {
        cout << "\n t=" << basics::typeName<T&>();
    }

    static void test()
    {
        //int v;
        f<int>();
        f<int&>();
        f<int&&>();

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
