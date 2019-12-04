#pragma once
#include "../common/basics.h"
#include <vector>
#include <new>   

namespace movetest
{
    using namespace std;

    class Base 
    {
    public:
        Base() :s_("default") {}
        Base(const std::string& s) :s_(s)
        {
            cout << "Base constructed from string" << endl;
            s_ = s;
        }
        // Base(const Base& other) = delete;
        Base(const Base& other)
        {
            cout << "Base::copy construct value" << endl;
            s_ = other.s_;
        }
        Base(Base&& other) noexcept
        {
            cout << "Base::move construct value" << endl;
            s_ = std::move(other.s_);
        }
        
        Base& operator= (Base&& other) noexcept
        {
            cout << "Base::move assing value" << endl;
            s_ = std::move(other.s_);
            return *this;
        }
        Base(const Base&& other)
        {
            cout << "Base::move const construct value" << endl;
            s_ = std::move(other.s_);
        }

        friend std::ostream& operator << (std::ostream& out, const Base& other);

        void f() {}
    private:
        string s_;
    };
    
    inline std::ostream& operator << (std::ostream& out, const Base& other)
    {
        out << other.s_.c_str();
        return out;
    }

    template <typename C>
    auto atIndex(C&& c, size_t index) -> decltype(std::forward<C>(c)[index])
    {
        //return std::forward<C>(c)[index];
        return std::forward<C>(c)[index];
    }

    template <typename C>
    decltype(auto) atIndex2(C&& c, size_t index)
    {
        //return std::forward<C>(c)[index];
        return c[index];
    }

    // ALIAS TEMPLATE _Unrefwrap_t
    template<class _Ty>
    struct _My_Unrefwrap_helper
    {	// leave unchanged if not a reference_wrapper
        using type = _Ty;
    };

    template<class _Ty>
    struct _My_Unrefwrap_helper<reference_wrapper<_Ty>>
    {	// make a reference from a reference_wrapper
        using type = _Ty & ;
    };

    // decay, then unwrap a reference_wrapper
    template<class _Ty>
    using _Unrefwrap_t = typename _My_Unrefwrap_helper<decay_t<_Ty>>::type;

    // FUNCTION TEMPLATE make_pair
    template<class _Ty1,
        class _Ty2>
        _NODISCARD constexpr pair<_Unrefwrap_t<_Ty1>, _Unrefwrap_t<_Ty2>>
        my_make_pair(_Ty1&& _Val1, _Ty2&& _Val2)
    {	// return pair composed from arguments
        basics::print(basics::typeName<_Ty1>());
        basics::print(basics::typeName<decay_t<_Ty1>>());
        using _Mypair = pair<_Unrefwrap_t<_Ty1>, _Unrefwrap_t<_Ty2>>;
        return (_Mypair(_STD forward<_Ty1>(_Val1), _STD forward<_Ty2>(_Val2)));
    }

    void test()
    {
        Base b1{"asd"};
        Base b2{"ttt"};
        auto p = my_make_pair(move(b1), move(b2));
        //auto p2 = my_make_pair(make_tuple(b1), make_tuple(b2));
        swap(b1, b2);
        //std::swap(1, 2);
        {
            vector<int> v1(10, 1);

           // decltype(v1[0]) t1 = 0;
            auto r = atIndex(v1, 2);
           // basics::print("v1 -> ", atIndex(v1, 2));
        }

        {
            const vector<Base> v1 = { Base("a"), Base("b"), Base("chhh") };
            auto&& r = atIndex(v1, 2);
            auto&& r10 = atIndex2(v1, 2);

            auto r3 = atIndex(v1, 2);
            auto& r2 = atIndex(vector<Base> { Base("a"), Base("b"), Base("chhh") }, 2);
            auto r4 = atIndex2(vector<Base> { Base("a"), Base("b"), Base("chhh") }, 2);
            auto r5 = atIndex2(v1, 2);
            auto& r6 = atIndex2(v1, 2);
           //basics::print("v1 -> ", atIndex(v1, 2));
        }
    }
}


