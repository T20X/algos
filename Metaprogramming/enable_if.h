#pragma once

/*
Removes/Adds function to overload resolution by hiding typedef type!

template<bool B, class T = void>
struct enable_if {};

template<class T>
struct enable_if<true, T> { typedef T type; };

Forms:
(1) typename = enable_if_t<is_same<Iter, random_access_iterator_tag>::value
*/

#include <iterator>
#include <iostream>
#include <vector>
#include <list>
#include <boost/type_index.hpp>
using namespace boost;
using namespace typeindex;



namespace enableif
{
    using namespace std;

    void testParam(int p, int)
    {
        void();
    }

    void testParam(double p, double)
    {
        void();
    }

    template <typename Container,
              typename Iter = std::iterator_traits<Container::iterator>::iterator_category,
              typename enable_if_t<is_same<Iter, random_access_iterator_tag>::value, int> = 0
 
    >
    void enableIfSimple(Container& v)
    {
        static_assert(is_same<Iter, random_access_iterator_tag>::value, "failed!");
        string s = type_id_with_cvr<enable_if_t<is_same<Iter, random_access_iterator_tag>::value>>().pretty_name();
        cout << "random interator " << s.c_str() << endl;
    }

    template <typename Container,    
        typename Iter = std::iterator_traits<Container::iterator>::iterator_category,       
        typename enable_if_t<is_same<Iter, bidirectional_iterator_tag>::value, int> = 1         
    >
        void enableIfSimple(Container& v)
    {
        static_assert(is_same<Iter, bidirectional_iterator_tag>::value, "failed!");
        string s = type_id_with_cvr<Iter>().pretty_name();
        cout << "bidirectional interator " << s.c_str() << endl;
    }



    template <typename Container,
        typename Iter = std::iterator_traits<Container::iterator>::iterator_category     
    >
        auto enableIfSINAFE(Container& v) -> decltype(enable_if_t<is_same<Iter, random_access_iterator_tag>::value>(), void())
    {
        static_assert(is_same<Iter, random_access_iterator_tag>::value, "failed!");
        string s = type_id_with_cvr<Iter>().pretty_name();
        cout << "random interator " << s.c_str() << endl;
    }

    template <typename Container,
        typename Iter = std::iterator_traits<Container::iterator>::iterator_category
    >
        auto enableIfSINAFE(Container& v) -> decltype(enable_if_t<is_same<Iter, bidirectional_iterator_tag>::value>(), void())
    {
        static_assert(is_same<Iter, bidirectional_iterator_tag>::value, "failed!");
        string s = type_id_with_cvr<Iter>().pretty_name();
        cout << "bidirectional interator " << s.c_str() << endl;
    }


    void f(void_t<>* p)
    {
        int o;
    }

    class A {
    public:
        int member1;
    };



    class C {
    public:
        int member;
    };


	// ALIAS TEMPLATE void_t
	template<class... _Types>
	using my_void_t = void;

    template<class T, class U = void>
    struct has_member : std::false_type
    { };
 
    // specialized as has_member< T , void > or discarded (sfinae)
    // IT HAS TO RESOLVE 2D INTO VOID or SFINAE FAILURE WILL USE THE PRIMARY TEMPLATE
    template<class T>
    struct has_member< T, my_void_t<decltype(T::member1)>> : std::true_type
    {};

	template<class T>
	struct has_member< T, int> : std::true_type
	{};

	namespace jpmx
	{
		struct A1
		{
		private:
			friend class B;
			int m;
		};
	}

	template <typename T, typename = void>
	struct is_iterable : std::false_type {};
	template <typename T>
	struct is_iterable<T, my_void_t<decltype(std::declval<T>().begin()),
									decltype(std::declval<T>().end())>>
		: std::true_type {};


	      
    void test()
    {
        testParam(12, int());
        testParam(12.4, double());
        vector<int> v;
        enableIfSimple(v);
        enableIfSINAFE(v);
        list<int> l;
        enableIfSimple(l);
        enableIfSINAFE(l);
        f(&v);

        static_assert(has_member<A>::value, "A");
		static_assert(has_member<C, int>::value, "B");
		static_assert(is_iterable<std::vector<int>>::value, "failed!");
        //static_assert(!has_member< B >::value, "B");
        //static_assert(has_member< C >::value, "C");
        //static_assert(has_member< B >::value, "B");

	


    }
}