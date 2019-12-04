#pragma once

namespace prob2
{
    /* was fun making it, but it is useless for the given task*/
    const uint64_t _INT_LIMIT_ = numeric_limits<uint64_t>::max();
    struct Factorials
    {
        map<int, uint64_t> factorials;
        uint64_t compute(int val)
        {
            uint64_t r = 1;
            int num = 0;
            {
                auto it = factorials.lower_bound(val);
                if (it != factorials.end())
                {
                    if (it->first == val)
                        return it->second;
                    else
                    {
                        r = it->second;
                        num = it->first;
                    }
                }
            }            
            
            while (++num <= val)
            {
                if (_INT_LIMIT_ / num < r)
                {
                    r = _INT_LIMIT_;
                    factorials.emplace(val, r);
                    break;
                }
                else
                {           
                    r *= num;
                    factorials.emplace(num, r);                    
                }
            }
            
            return r;
        }
    };

    const int _MAX_ = 1'000'000'000;
    int solution(const vector<int>& A)
    {
        unordered_map<int, int> m;
        for (auto num : A)
            ++m[num];

        int N = 0;
        for (const auto& p : m)
        {
            int sameN = p.second;
            if (sameN == 2)
                ++N;
            else if (sameN > 2)
            {
                //just really combinations
                int local = sameN * (sameN - 1) / 2;
                if (local >= _MAX_)
                 return _MAX_;
                N += local;
            }

            if (N >= _MAX_)
              return _MAX_;
        }

        return N;
    }

    static void test()
    {
        cout << "\n----------------PROB2--------------";
        cout << "\n m=" << numeric_limits<int>::max();
        try
        {
            assert(solution({ 3,5,6,3,3,5 }) == 4);
            assert(solution({ 3,5,6,3,3,5,3,3 }) == 11);
            assert(solution({ 3 }) == 0);
            assert(solution({ 3,3 }) == 1);
            assert(solution({ 3,4 }) == 0);
        }
        catch (const std::exception & e)
        {
            cout << e.what();
        }

        Factorials f;
        assert(f.compute(1) == 1);
        assert(f.compute(2) == 2);
        assert(f.compute(3) == 6);
        assert(f.compute(300) == _INT_LIMIT_);
        for (int i = 0; i < 200; ++i)
        {
            cout << "\n i=" << i << " f=" << f.compute(i);
            if (i > 2)
                cout << " N=" << f.compute(i) /
                                 f.compute(i - 2);
        }
        cout << "\n----------------PROB2--------------";
    }
}

