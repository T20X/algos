#pragma once

namespace prob1
{    
    string solution(string& S)
    {
        if (S.empty())
            return S;
            
        auto candidateIt = prev(S.end());
        for (auto sIt = next(S.begin()); sIt != S.end(); ++sIt)
        {
            if (*sIt < *prev(sIt))
            {
                candidateIt = prev(sIt);
                break;
            }         
        }

        S.erase(candidateIt);
        return S;
    }

    static void test()
    {
        using namespace basics;
        string s1("hot");
        p(solution(s1));
        string s2("acb");
        p(solution(s2));
        string s3("aaaa");
        p(solution(s3));
        string s4("codility");
        p(solution(s4));
        string s5("abcde");
        p(solution(s5));
        string s6("");
        p(solution(s6));
    }
}
