#pragma once

namespace factory
{
    using namespace std;

    class Base
    {
    public:
        virtual void f()
        {
            cout << "\n Base::f";
        }

        virtual Base* clone() = 0;
        virtual ~Base() {}
    };

    template <class Derived>
    class Cloner : public virtual Base
    {
    public:
        Base* clone() {
            return new Derived(
                *static_cast<Derived*>(this));
        }
    };

    class Derived : public Cloner<Derived>
    {
    public:
        void f() override
        {
            cout << "\n Derived::f";
        }
    };

    class Derived2 : public Derived
    {
    public:
        void f() override
        {
            cout << "\n Derived2::f";
        }
    };

    static void test()
    {
        Derived d;
        Base* b = d.clone();
        b->f();
        Base* b1 = b->clone();
        b1->f();
        Base* b2 = new Derived2();
        b2->clone()->f();
    }
}
