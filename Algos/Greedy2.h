#pragma once

#include <vector>
#include <iostream>
#include <limits>
#include <algorithm>

using namespace std;

int finTime(vector<int>::const_iterator& it, const vector<int>& v)
{
    if (it == v.end())
    {
        return numeric_limits<int>::max();
    }
    else
    {
        return *it + 1000;
    }
}

int examine(vector<int>::const_iterator& it, const vector<int>& v, int minFinTime)
{
    if (it == v.end() || *it > minFinTime)
    {
        return 0;
    }
    else
    {
        it++;
        return 1;
    }
}

class Boxing
{
public:
    int maxCredit(vector <int> a, vector <int> b, vector <int> c, vector <int> d, vector <int> e)
    {
        if (a.size() > 50 ||
            b.size() > 50 ||
            c.size() > 50 ||
            d.size() > 50 ||
            e.size() > 50)
        {
            return 0;
        }

        int N = 0;

        auto aIt = a.cbegin();
        auto bIt = b.cbegin();
        auto cIt = c.cbegin();
        auto dIt = d.cbegin();
        auto eIt = e.cbegin();

        int lastMinFinTime = numeric_limits<int>::max();

        while (aIt != a.end() ||
            bIt != b.end() ||
            cIt != c.end() ||
            dIt != d.end() ||
            eIt != e.end())
        {
            int minFinTime = numeric_limits<int>::max();
            int localSolution = 0;
            {                
                if (finTime(aIt, a) < minFinTime)
                    minFinTime = finTime(aIt, a);

                if (finTime(bIt, b) < minFinTime)
                    minFinTime = finTime(bIt, b);

                if (finTime(cIt, c) < minFinTime)
                    minFinTime = finTime(cIt, c);

                if (finTime(dIt, d) < minFinTime)
                    minFinTime = finTime(dIt, d);

                if (finTime(eIt, e) < minFinTime)
                    minFinTime = finTime(eIt, e);

                localSolution += examine(aIt, a, minFinTime);
                localSolution += examine(bIt, b, minFinTime);
                localSolution += examine(cIt, c, minFinTime);
                localSolution += examine(eIt, e, minFinTime);
                localSolution += examine(dIt, d, minFinTime);
            }

            if (localSolution >= 3 && lastMinFinTime != (minFinTime - 1000))
            {
                lastMinFinTime = minFinTime;
                N++;
            }
        }

        return N;
    }

};

class GreedyChildren1
{
public:
    int findContentChildren(vector<int>& g, vector<int>& s)
    {
        std::sort(g.begin(), g.end());
        std::sort(s.begin(), s.end());

        int fedChildrenN = 0;

        int k = 0, c = 0;
        while (k < g.size() && c < s.size())
        {
            if (g[k] <= s[c])
            {
                k++;
                fedChildrenN++;
            }

            c++;
        }

        return fedChildrenN;
    }
};

class GreedyChildren2
{
public:
    int findContentChildren(vector<int>& g, vector<int>& s)
    {
        int j = 0;
        for (auto sVal : s)
        {
            int minGval = -1;
            for (auto it = g.begin() + j; it != g.end(); it++)
            {
                if (*it > minGval) minGval = *it;
            }  
        }
    }
};

void testBoxing2()
{
    vector<int> v;
    v.push_back(12);

    vector<int> v1;
    v.push_back(15);

    v.insert(v.end(), 12);
    v.insert(v.begin(), 1);
    v.insert(v.begin(), 2);

    auto it = v.begin() + 1;
   // v.insert(v.begin() + 1, 5);
    //cout << *it << endl;

    // Test #1
    {
        vector<int> a = { 100, 200, 300, 1200, 6000 };
        vector<int> b = { 900, 902, 1200, 4000, 5000, 6001 };
        vector<int> c = { 0, 2000, 6002 };
        vector<int> d = { 1, 2, 3, 4, 5, 6, 7, 8 };
        vector<int> e = {};

        Boxing box;
        cout << box.maxCredit(a, b, c, d, e) << endl;
    }

    // Test #2
    {
        Boxing box;
        cout << box.maxCredit({ 1, 2, 3, 4, 5, 6 },
        { 1, 2, 3, 4, 5, 6, 7 },
        { 1, 2, 3, 4, 5, 6 },
        { 0, 1, 2 },
        { 1, 2, 3, 4, 5, 6, 7, 8 })
            << endl;

    }

    // Test #3
    {
        Boxing box;
        cout << box.maxCredit({ 5000, 6500 },
        { 6000 },
        { 6500 },
        { 6000 },
        { 0, 5800, 6000 })
            << endl;
    }

    // Test #4
    {
        Boxing box;
        cout << box.maxCredit({ 0 },
        { 0 },
        { 0 },
        { 0 },
        { 0 })
            << endl;
    }
}

