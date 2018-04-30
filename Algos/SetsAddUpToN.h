#pragma once

#include <vector>

class SetsAddUpToN
{
public:
    int solve(vector<int>& v, int N)
    {   
        int solution = 0;
        int workingIndex = v.size() - 1;
        solution += solveAux(v, N, workingIndex);
        return solution;
    }

    int solveAux(const vector<int>& v, int N, int index)
    {
        if (N == 0)
            return 1;

        if (N < 0 || index < 0)
            return 0;

        return solveAux(v, N, index - 1) +
               solveAux(v, N - v[index], index - 1);
    }

    void test()
    {
        {
            vector<int> v = { 16 };
            cout << "Result -> " << solve(v, 16) << endl;
        }

        {
            vector<int> v = { 2, 16, 6, 10, 4, 26, 39, 100,12 };
            cout << "Result -> " << solve(v, 16) << endl;
        }
    }
};