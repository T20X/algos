#pragma once
#include <type_traits>

namespace typelist
{
    template <class T>
    struct just_type { using type = T; };

    template <class... T>
    struct type_pack {};

    template <template <class...> class F>
    struct fn_type
    {
        template <class... Ts>
        constexpr auto operator()(just_type<Ts>...) {
            return F<Ts...>::type{};
        }
    };

    template <class F, class... Ts>
    constexpr auto transform(F f, type_pack<Ts...>) {
        return type_pack<decltype(f(just_type<Ts>{}))... > {};
    }

   // template <template <class...> class F>
   //  constexpr fn_type<F> fn_t;

    static_assert(std::is_same<
        decltype(transform(fn_type<add_pointer>(), type_pack<int, int, double>())),
         type_pack<int*, int*, double*>
     >::value, "sdf");

    struct A { void p() { std::cout << "\n A; "; } };
    struct B { void p() { std::cout << "\n B; "; } };
    struct C { void p() { std::cout << "\n C; "; } };
    template <class E> struct Handle { void process(E& e) { e.p(); } };

    template <class... EL>
    struct Expectation : Handle<EL>...
    {
        template <class E>
        void _p(E& e) { Handle<E>::process(e); }
    };

    static void test()
    {
        Expectation<A, B, C> e;
        A a{};
        e._p(a);
    };

}
