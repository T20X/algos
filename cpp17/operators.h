#pragma once

namespace operators
{
    struct object
    {
        object(int k)
        {
            cout << "\n object::object";
        }
        object(const object& other)
        {
            cout << "\n object::copy";
        }
        object& operator=(const object& other)
        {
            cout << "\n object::=";
            return *this;
        }
    };

    object f() { return 2; }
    static void test()
    {
        object o1 = f(); //only constructor is called
        object o2 = 1;
        o2 = f();
    }
}
