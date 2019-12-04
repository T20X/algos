#pragma once

#include "Spinlock.h"
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>

namespace tests
{
    std::atomic<bool> guard(false);
    std::atomic_flag guard2 = ATOMIC_FLAG_INIT;

    void testSpinLockAtomicBool()
    {                
        constexpr static int LEN = 1000000;
        constexpr static int THREADS_N = 5;

        int sums = 0;
        auto counter = [&] {
            for (int i = 0; i < LEN; i++)
            {
                ScopedSpinlock2 autolock(guard);
                sums++;
            }
        };

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        std::vector<std::thread> pool;
        for (int i = 0; i < THREADS_N; i++)
            pool.emplace_back(counter);

        for (auto& t : pool)
            t.join(); 

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        cout << "\nTook ->" << (double)std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() 
            << " millis per item" << std::endl;

        std::cout << "\n sums -> " << sums
            << " expected-> " << LEN * THREADS_N << "\n\n";
    }

    void testSpinLockAtomicFlag()
    {
        constexpr static int LEN = 1000000;
        constexpr static int THREADS_N = 5;

        int sums = 0;
        auto counter = [&] {
            for (int i = 0; i < LEN; i++)
            {
                ScopedSpinlock autolock(guard2);
                sums++;
            }
        };

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        std::vector<std::thread> pool;
        for (int i = 0; i < THREADS_N; i++)
            pool.emplace_back(counter);

        for (auto& t : pool)
            t.join();

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        cout << "\nTook ->" << (double)std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
            << " millis per item" << std::endl;

        std::cout << "\n sums -> " << sums
            << " expected-> " << LEN * THREADS_N << "\n\n";
    }
}
