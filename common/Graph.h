#pragma once

#include <unordered_map>
#include <unordered_set>
#include <list>
#include <memory>
#include <exception>
#include <string>
#include <vector>
#include <assert.h>
#include <stdexcept>
#include <iostream>


namespace graph
{
    template <class Vertex, class Hash=std::hash<Vertex>, class Weight=int>
    struct Graph
    {
        bool directed_;
        using Edge = std::pair<Vertex, Weight>;
        std::unordered_set<Vertex, Hash> vertecies_;
        std::unordered_map<Vertex, std::list<Edge>, Hash> g;
        size_t edgeSize_ = 0;
        explicit Graph(bool directed)
            :directed_(directed)
        {
        }

        template <class F>
        void forEachVertex(const F& visitor) const
        {
            for (const auto& v : vertecies_)
                visitor(v);
        }

        template <class F>
        void forEachEdge(const F& visitor) const
        {
            for (const auto& v : g)            
                for (const auto& p : v.second)
                    visitor(v.first, p.first, p.second);            
        }

        bool containsVertex(const Vertex& v) const
        {
            auto it = vertecies_.find(v);
            return !(it == vertecies_.end());
        }

        bool containsAdj(const Vertex& v) const
        {
            auto it = g.find(v);
            return !(it == g.end());
        }

        const std::list<Edge>& adj(const Vertex& v) const
        {
            auto it = g.find(v);
            if (it == g.end()) {};
            return it->second;
        }

        auto vertexSize() const { return vertecies_.size(); }
        auto edgeSize() const { return edgeSize_; }

        bool addEdge(const Vertex& from, const Vertex& to, const Weight& w = {})
        {
            if (!directed_ && from == to)
                return false;

            std::list<Edge>& adj = g[from];
            auto it = std::find_if(begin(adj), end(adj),
               [&to](const auto& v) { return v.first == to; });
            if (it != adj.end()) return false;

            adj.emplace_back(Edge( to,w ));
            ++edgeSize_;

            if (!directed_) addEdge(to, from, w);
             vertecies_.emplace(from);
             vertecies_.emplace(to);
            return true;
        }
    };

    auto intStrtoGraph(const std::vector<std::string>& s, bool d)
    {
        std::unique_ptr<Graph<int>> g(new Graph<int>(d));

        for (const auto& adj : s)
        {
            size_t pos = adj.find_first_of(':', 0);
            if (pos == std::string::npos) throw std::runtime_error("no :!");
            int v = std::stoi(adj.substr(0, pos));

            //[front;back)
            int front = pos+1;
            int back = front+1;            
            if (front == adj.size()) throw std::runtime_error("no adj!");
            while (front < adj.size())
            {   
                back = adj.find_first_of(',', front);
                if (back == std::string::npos)
                    back = adj.size();
                    
                if (front <= back)
                {
                    g->addEdge(v, std::stoi(adj.substr(
                       front, back - front)));
                }
                else
                    throw std::runtime_error("out of bound!");

                back += 1;
                front = back;                
            }
        }

        return g;
    }

    auto intStrtoGraphW(const std::vector<std::string>& s, bool d)
    {
        std::unique_ptr<Graph<int>> g(new Graph<int>(d));

        for (const auto& adj : s)
        {
            size_t pos = adj.find_first_of(':', 0);
            if (pos == std::string::npos) throw std::runtime_error("no :!");
            int v = std::stoi(adj.substr(0, pos));

            //[front;back)
            int front = pos + 1;
            int back = front + 1;
            if (front == adj.size()) throw std::runtime_error("no adj!");
            while (front < adj.size())
            {                
                back = adj.find_first_of('(', front);
                if (back == std::string::npos)
                    throw std::runtime_error("no parantesis (");

                int to;
                if (front <= back)                
                    to=std::stoi(adj.substr(front, back - front));                
                else
                    throw std::runtime_error("out of bound!");

                ++back;
                front = back;
                if (front < adj.size())
                {
                    back = adj.find_first_of(')', front);
                    if (back == std::string::npos)
                        throw std::runtime_error("no parantesis (!");
                    
                    int w = std::stoi(adj.substr(front, back - front));
                    if (back < adj.size()-1 && adj[back+1] != ',')
                        throw std::runtime_error("missing ,");

                    back += 2;
                    front = back;
                    g->addEdge(v, to, w);
                }
                else
                    throw std::runtime_error("out of bound!");
            }
        }

        return g;
    }

    static void testIntStrtoGraph()
    {
        using namespace std;

        { vector<string> s = {"1:"};
          try { intStrtoGraph(s, true);  assert(false); } catch (...) {} }
        { vector<string> s = { "" };
          try { intStrtoGraph(s, true);  assert(false); } catch (...) {} }
        { vector<string> s = { "1:," };
          try { intStrtoGraph(s, true);  assert(false); } catch (...) {} }        
        { vector<string> s = { "1:1,,," };
          try { intStrtoGraph(s, true);  assert(false); } catch (...) {} }
        { 
          vector<string> s = { "1:2" };
          try { 
            auto g = intStrtoGraph(s, true); 
            assert(g->vertexSize() == 2);
          }
          catch (const std::exception& v) { std::cout << "\n exp=" << v.what();  assert(false); }
        }
        {
            vector<string> s = { "1:2,3", "2:1,5,4"};
            try {
                auto g = intStrtoGraph(s, true);
                assert(g->vertexSize() == 5);
                assert(g->edgeSize() == 5);
            }
            catch (const std::exception& v) { std::cout << "\n exp=" << v.what();  assert(false); }
        }
        {
            vector<string> s = { "1:2,13", "2:1,500,14" };
            try {
                auto g = intStrtoGraph(s, false);
                assert(g->vertexSize() == 5);
                assert(g->edgeSize() == 8);
            }
            catch (const std::exception& v) { std::cout << "\n exp=" << v.what();  assert(false); }
        }
        {
            vector<string> s = { "1:2(10),13(12)", "2:1(56),500(67),14(32)" };
            try {
                auto g = intStrtoGraphW(s, false);
                assert(g->vertexSize() == 5);
                assert(g->edgeSize() == 8);
            }
            catch (const std::exception& v) { std::cout << "\n exp=" << v.what();  assert(false); }
        }
    }
}
