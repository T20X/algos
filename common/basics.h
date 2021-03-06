#pragma once

#include <boost/type_index.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <type_traits>

namespace basics
{

    template <typename deleted_type>
    void del(deleted_type*& p)
    {
        if (&p)
        {
            delete p;
            p = nullptr;
        }
    }

    template <typename T>
    std::string typeName()
    {
        return boost::typeindex::type_id_with_cvr<T>().pretty_name();
    }

    template <typename T>
    void print1(const T& arg)
    {
        std::cout << arg << "|";
    }

    template <typename... T>
    void  print(const T&... args)
    {
        std::cout << std::endl;
        int dummy[] = { 0, ((void)print1(args), 0) ... };
        std::cout << std::endl;
    }

    void printX()
    {    
    }

    template <typename T>
    void printX(const T& arg)
    {
        print1(arg);
    }

    template <typename F, typename... T>
    void  printX(const F& first, const T&... args)
    {        
        print1(first);
        printX(args...);        
    }

    template <typename... T>
    void printV2(const T&... args)
    {   
        std::cout << std::endl;
        printX(args...);
        std::cout << std::endl;
    }

    using namespace std;

    template <typename... T>
    void p(const T&... args)
    {
        int a[] = { 0, ((void)(cout << "," << args), 0)... }; (void)a;
        cout << "\n";
    }

    struct Out
    {
        template <typename... Args>
        static auto perfect_capture(Args&&... args) {
            return std::tuple<Args...>(std::forward<Args>(args)...);
        }

        template <class... Args, size_t... I>
        void printTuple(std::ostream& out, const std::tuple<Args...>& data, std::index_sequence<I...>) {
            out << "\n";
             int a[] = { 0, ((void)(out << "," << get<I>(data)), 0)... }; (void)a;
            out << "\n";
        }

        template <class... T>
        Out(T&&... args) {             
            _print = [this,data=perfect_capture(std::forward<T>(args)...)](auto& out) {
                printTuple(out, data, std::make_index_sequence<sizeof...(args)>{});                                               
            };
        }
    
        friend std::ostream& operator<<(std::ostream&, Out&);
        std::function<void(std::ostream&)> _print;
    };

    std::ostream& operator<<(std::ostream& out, Out& other)    {
        other._print(out);
        return out;
    }

    template <typename C> using ContainerType = decltype(*begin(declval<C>()));
    template <typename C, typename = void_t<>> struct is_container : std::false_type {};
    template <typename C> struct is_container<C,
        void_t<decltype(*begin(declval<C>())),
        decltype(*end(declval<C>())),
        typename C::iterator
        >> : std::true_type {};

    static_assert(is_container<vector<size_t>>::value, "not vec");

    template <typename C, typename = enable_if_t<
        std::conjunction<
        is_container<C>,
        std::negation<is_same<C, string>>
        >::value
        >>
        ostream & operator<<(ostream& out, const C& v)
    {
        for (const auto& item : v)
            cout << item << ",";
        return out;
    }
}

