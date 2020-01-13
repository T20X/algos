#pragma once

namespace concepts
{
    template <class C>
    constexpr auto func(C&& v, int) -> decltype(v.require(), void()) {
       // return {};       
    }

    template <class C>
    constexpr void func(C&& v, ...) { 
        static_assert(false,"Type does not satisfy concept!");      
        C::_____valid_expression;
    }

    template <class T>
    constexpr decltype(auto) _func(T&& t) {
        func(forward<T>(t), 0);
    }


    template<class T>
    struct C1
    {
        constexpr int require()
        {
            _func(isDouble<T>);
            //cout << "\n hello fuckers";
        }
    };

    template<class T>
    struct isInteger
    {
    };

    template <>
    struct isInteger<int>
    {
        int require() {}
    };

    template<class T>
    struct isDouble
    {
    };

    template <>
    struct isDouble<double>
    {
        void require() {}
    };


    static void test()
    {
        _func(isInteger<int>());
      //  _func(C1<int>());
       // _func(isInteger<double>());
        //func(C1());
    }

#include <utility>
#include <string>

    template <class C>
    constexpr auto func2(C v, int) -> decltype(v.require(), void()) {
        v.require();
    }

    template <class C>
    constexpr void func2(C v, ...)
    {
        C::_____valid_expression();
        //static_assert(false,"Type does not satisfy concept!");            
    }

    template <class T>
    constexpr void _func2(T t) {
        func(t, 0);
    }


    //#include <iostream>
    template<class T>
    struct isDouble2
    {
    };

    template <>
    struct isDouble2<double>
    {
        constexpr void require() {}
    };

    template<class T>
    struct C12
    {
        template <class T1 = T>
        constexpr void require()
        {
            _func(isDouble<T1>());
            // std::string s1;           
            //std::cout << "\n hello fuckers";
        }
    };

    template<class T>
    struct isInteger2
    {
        //constexpr isInteger() {};
    };

    template <>
    struct isInteger2<int>
    {
        //constexpr isInteger() = default;
        constexpr void require() {}
    };

    template<class T>
    struct S2
    {
    };

    constexpr int _func12(isInteger2<int> t)
    {
        //std::string s1;     
        return 10;
    }

    static void test2()
    {
        //static_assert(std::is_literal_type<decltype(isInteger2<int>())>::value);
        constexpr int v = _func12(isInteger2<int>());
        int ar[v];
        ar[2] = 11;
        constexpr bool v1 = noexcept(_func12(isInteger2<int>()));
        //static_assert(v1,"v1 not constexpr");
        constexpr bool v2 = noexcept(_func2(C12<double>()));
        // static_assert(v2,"v2 not constexpr");
        constexpr bool v3 = noexcept(isInteger2<int>());
        static_assert(v3, "v3 not constexpr");
        _func(C12<int>());
        //_func(isDouble<int>());
        //func(C1());
    }


}
