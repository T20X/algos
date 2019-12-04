#pragma once

#include <iterator>
#include <iostream>
#include <vector>
#include <list>
#include <boost/type_index.hpp>


using namespace std;
using namespace boost;
using namespace typeindex;

namespace specialization
{
	template<typename T>
	class A
	{
		static constexpr double p = 1.5;
		int x;
	}; // primary template

	template<typename U, int N>
	class A < U[N] >
	{
		static constexpr double p = 1.5;
		int x[N];
	}; // primary template

    template<typename T, typename O = int>
    class A1
    {
		static constexpr double p = 1.5;
        int x;
    }; // primary template

    template<class T, typename U>
    class A1<T*, U*>
    {
    public:
        long x;
        // U* l;
        T* p;
        int j;
    }; // partial specialization

    template <class T, class U>
    class M
    {
        int x;
    };

    template <class T>
    class M<T, int>
    {
        double x;
    };

    inline void test()
    {
		
        A1<double*, double*> a13; //a13.l = nullptr;
        A1<double> a11;
        A1<decltype(declval<double>())> a12;

        M<int, int> l1;
        M<int, double> l2;
    }
}

