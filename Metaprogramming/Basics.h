#pragma once

#include <vector>
#include <list>
#include <type_traits>
#include <tuple>
#include <map>
#include <unordered_map>
#include <memory>
#include <utility>

#include "../common/basics.h"

namespace basics
{    
    struct X
    {
        enum { value1 = true, value2 = true };
    };

    template<bool _Test,
        class _Ty = int>
        struct enable_if_mine
    {	// type is undefined for assumed !_Test
    };

    template<class _Ty>
    struct enable_if_mine<true, _Ty>
    {	// type is _Ty for _Test
        using type = _Ty;
    };

    template <typename = enable_if_mine<X::value1>::type>
    void showme()
    {
        std::cout << "worked" << std::endl;
    }

    template<class T, std::enable_if_t<T::value1, int> = 0>
    void func() {} // #1

    template void func<X>(); // #2

    template <typename C>
    constexpr bool is_random_iterator = is_convertible<
        typename C::iterator::iterator_category,
        random_access_iterator_tag>::value;

    template <typename C, typename std::enable_if<        
        is_random_iterator<C>, bool>::type = 0>
        void rev(C& c)
    {
        cout << "random iterator reverse 1 " << endl;
    }

    template <typename C, class = typename std::enable_if<
        !is_random_iterator<C>, size_t>::type>
        void rev(C& c)
    {

        cout << "other iterator reverse 1 " << endl;
    }



   /* template <typename C, class = !std::enable_if_t<
        is_same<typename C::iterator::iterator_category, random_access_iterator_tag>::value>>
        void rev(C& c)
    {

        cout << "other iterator reverse 1 " << endl;
    }*/
    

    template <class ItType, class ItCategory = typename iterator_traits<ItType>::iterator_category>
    int mine_distance(ItType back, ItType front)
    {
        return _aux_distance(back, front, ItCategory());
    }

    template <class ItType>
    int _aux_distance(ItType back, ItType front, random_access_iterator_tag)
    {
        return back - front;
    }

    template <class ItType>
    int _aux_distance(ItType back, ItType front, input_iterator_tag)
    {
        int result = 0;
        while (back != front)
        {
            result++;
            --back;
        }

        return result;
    }


    /*template <typename C, class = std::enable_if_t<
        conjunction<
        is_same<typename C::iterator::iterator_category, random_access_iterator_tag>
         >::value>>
     void rev(C& c)
    {
        
        cout << "random iterator reverse 1 " << endl;        
    }*/

    /*template <typename C, class = std::enable_if_t<
        conjunction<
        is_same<typename C::iterator::iterator_category, bidirectional_iterator_tag>
        >::value>>
        void rev(C& c)
    {

        cout << "bidirectional iterator reverse 1 " << endl;
    }*/

    /*template <typename C, std::enable_if_t<
        is_same<typename C::iterator::iterator_category, random_access_iterator_tag>::value, int> = 0>
        void rev(C& c)
    {

        cout << "random iterator reverse 5 " << endl;
    }
    
    template <typename C, std::enable_if_t<
        is_same<typename C::iterator::iterator_category, bidirectional_iterator_tag>::value, int> = 0>
        void rev(C& c)
    {

        cout << "bidirectional iterator reverse 5 " << endl;
    }*/


    template <typename...>
    using void_t = void;

    template <typename C, 
        typename = void> 
        struct is_iterable 
        : std::false_type {}; 

        template <typename C> 
        struct is_iterable< 
        C, void_t<decltype(*begin(std::declval<C>())), 
        decltype(end(std::declval<C>()))>> 
        : std::true_type{};

    template <typename T1, typename T2>
    struct mine_is_same : std::false_type {};

    template <typename T>
    struct mine_is_same<T,T> : std::true_type {};

    template <typename T>
    using contained_type_t =
        std::remove_cv_t < std::remove_reference_t<
        decltype(*begin(std::declval<T>()))
        >>;

    template <typename T> 
        struct remove_reference {       
            using type = T; 
    }; 

    template <typename T> 
       struct remove_reference<T&> {
           using type = T; 
     }; 

    template <typename T> 
       struct remove_reference<T&&> {                
           using type = T; 
    };

    template <typename T>
     using remove_reference_t = typename remove_reference<T>::type;

    template <typename C,
              typename R = contained_type_t<C>>
    R sum(const C& c)
    {
        return R();
    }

    template <typename Function, typename... CapturedArgs>
    class curried {
    private:
        using CapturedArgsTuple = std::tuple<
            std::decay_t<CapturedArgs>...>;

        template <typename... Args>
        static auto capture_by_copy(Args&&... args)
        {
            return std::tuple<std::decay_t<Args>...>(
                std::forward<Args>(args)...);
        }
    public:
        curried(Function function, CapturedArgs... args)
            : m_function(function)
            , m_captured(capture_by_copy(std::move(args)...))
        {
        }
        curried(Function function, std::tuple<CapturedArgs...> args)
            : m_function(function)
            , m_captured(std::move(args))
        {
        }
       
        template <typename... NewArgs>
       auto operator()(NewArgs&&... args) const
        {
                    auto new_args = capture_by_copy(
                    std::forward<NewArgs>(args)...); 

                    auto all_args = std::tuple_cat(
                        m_captured, std::move(new_args)); 

                                         
                   // return std::apply(m_function, all_args); 
                    return 10;
              
        }

    private:
        Function m_function;
        std::tuple<CapturedArgs...> m_captured;
    };

    int t1(int a1, int b1, int c1)
    {
        return a1 + b1 + c1;
    }



    template<class...> 
    struct conjunction : std::true_type { };
    
    template<class B1> 
      struct conjunction<B1> : B1 { };

    template<class B1, class... Bn>
    struct conjunction<B1, Bn...>
        : std::conditional_t<bool(B1::value), conjunction<Bn...>, B1> {};

    // otherwise
    template<typename T, typename... Ts>
    std::enable_if_t<!std::conjunction<std::is_same<T, Ts>...>::value>
        func(T, Ts...) {
        std::cout << "not all types in pack are T\n";
    }

    template<class _Iter>
    struct RemoveRvalue
    {	// construct unchecked from checked, generic
        using type = _Iter;
    };

    RemoveRvalue<int> uniformInitialization2()
    {
        return {}; //equal to RemoveRvalue<int>{}
    }
    void testDeclval()
    {
        struct DeclvalStruct
        {
            DeclvalStruct(int x, int y):_x(x),_y(y) {}
            int _x = 0;
            int _y = 0;

            ~DeclvalStruct()
            {
                basics::print("decvaldestructor ");
            }
        };


        using ftype2 = RemoveRvalue<decltype(declval<DeclvalStruct>())>::type;
        basics::print(basics::typeName<ftype2>());
        using ftype = std::remove_reference<decltype(declval<DeclvalStruct>())>::type;
        ftype s = { 1, 2 };
        basics::print(s._x, s._y);
        basics::print(basics::typeName<ftype>());
        DeclvalStruct&& s1{ 4,7 };
        basics::print(s1._x, s1._y);
    }

    void test()
    {
        showme();
        /*curried<decltype(&t1), int, int> f1(&t1, 10, 10);
        int ret = f1(10);
        if (ret == 30)
            cout << endl << "worked";*/
        testDeclval();
        void_t<int, int, int>* f = 0;
        vector<int> v = { 1, 2, 5, 10, 2 };
        int r1 = mine_distance(v.begin() + 3, v.begin());
        list<int> l = { 1, 2, 5, 10, 2 };
        r1 = mine_distance(l.end(), l.begin());
        rev(v);
        rev(l);
       // rev(l);
        int r = sum(v);       
        int i1 = 1;
        int& i2 = i1;
        remove_reference_t<decltype(i2)> t;
        cout << "#1 -> the same " << std::is_same<remove_reference_t<decltype(i2)>, int>::value;
        tuple<int, int, double> t1;

		bool isIterable = is_iterable<int>::value;
        //func(1, 2, 3);
        func(1, 2, "hello!");
        

    }
}
