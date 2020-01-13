#pragma once
#include <string>

namespace func
{
    struct FilterByName
    {
        struct person_t
        {
            std::string name;
            //person_t(const std::string& v) :name(v) {}
        };


        template <typename C, typename F, typename E, typename R>
        static auto filter1(C collection, F func, E extract, R& result) -> void {
            if (collection.empty())
                return;

            auto head = extract(collection.front());
            if (func(head)) {                
                result.emplace(end(result), head);
            }
            
            collection.erase(begin(collection));
            filter1(collection, func, extract, result);
        }

        static void test() {
            vector<person_t> v{
                person_t{"T"},
                person_t{"T"},
                person_t{"P"},
                person_t{"T"},
                person_t{"P"}
            };
            vector<string> r;
            filter1(v,[&](const auto& v)->bool{return v=="T";},
                      [&](const auto& v)->string {return v.name; }, 
                      r);
            assert(r.size() == 3);
            assert(r.front() == "T");
            assert(r.back() == "T");

        }
    };

    static void test() {
        FilterByName::test();
    }
}
