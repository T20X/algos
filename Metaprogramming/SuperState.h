#pragma once

namespace superstate
{
    template <class T>
    struct just_type { using type = T; };

    template <class... T>
    struct type_pack {};

    template <class... T>
    constexpr size_t type_pack_size(type_pack<T...>) { return sizeof...(T); }

    struct Superstate
    {

    };

    static void test()
    {
    }
}