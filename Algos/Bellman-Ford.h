#pragma once
#include "../common/Graph.h"
namespace bellman
{
    template <class V, class W=int>
    struct BellmanFord
    {
        using Result = std::unordered_map<V, W>;
        template <typename Graph, typename Vertex>
        Result operator()(const Graph& g, const Vertex& ve)
        {
            Result r;
            r.emplace(ve,0);
            g.forEachVertex([&](const auto& v) {
                g.forEachEdge([&](const auto& from, const auto& to, auto weight) {
                    auto itF = r.find(from);
                    if (itF == r.end())
                        return;
                    int d = itF->second + weight;
                    auto itT = r.find(to);
                     if (itT == r.end()) r[to] = d;
                    else itT->second = min(itT->second, d);
                  });
                });

            bool negativeCycle = false;
            g.forEachEdge([&](const auto& from, const auto& to, auto weight) {
                if (r[to] > r[from] + weight) negativeCycle=true; });
            return negativeCycle ? Result() : r;
        }
    };

    using namespace std;
    static void test()
    {        
        {
            vector<string> str = {
                "1:4(10),5(80),6(3)",
                "4:5(5)",
                "5:9(5)"
            };

            auto g = graph::intStrtoGraphW(str, false);
            auto r = BellmanFord<int>()(*g, 1);
            cout << "\n r[9]=" << r[9];
            assert(r[5] == 15);
            assert(r[9] == 20);
            assert(r[0] == 0);
        }

        {
            vector<string> str = {
                "1:4(-10),5(-80),6(3)",
                "4:5(-5)",
                "5:9(5)"
            };

            auto g = graph::intStrtoGraphW(str, false);
            auto r = BellmanFord<int>()(*g, 1);            
            assert(r.empty());            
        }
    }
}
