#pragma once

#include "SPSCConflatedQueue.h"
#include <functional>
#include <thread>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <unordered_set>

using namespace std;

namespace tests
{    
    void testSPSCConflation()
    {
        constexpr static size_t SAMPLES_N = 1000000;
        static int moveN = 0, moveAssingN = 0, copyN = 0, copyAssingN = 0;

        {
            struct NoDefaultConstructible
            {
                int i;
                NoDefaultConstructible(int i_) :i(i_) {}
                NoDefaultConstructible(NoDefaultConstructible&& other)
                {
                    i = other.i;
                    moveN++;
                }
                NoDefaultConstructible& operator= (NoDefaultConstructible&& other)
                {
                    if (this != &other)
                    {
                        i = other.i;
                        moveAssingN++;
                    }

                    return *this;
                }
                NoDefaultConstructible(const NoDefaultConstructible& other)
                {
                    i = other.i;
                    copyN++;
                }
                NoDefaultConstructible& operator= (const NoDefaultConstructible& other)
                {
                    if (this != &other)
                    {
                        i = other.i;
                        copyAssingN++;
                    }
                    
                    return *this;
                }
            };

            int conflatedN = 0;
            int updatesN = 0;
        
            atomic<bool> finished(false);

            SPSCQueueConflated<NoDefaultConstructible> q(2);

            thread p([&q, &conflatedN, &finished]() {

                unordered_map<uint64_t, SPSCQueueConflated<NoDefaultConstructible>::Proxy::Ptr> updates;
                for (int i = 0; i < 10; i++)
                {
                    updates.emplace(i, new SPSCQueueConflated<NoDefaultConstructible>::Proxy(i * 10, nullptr ));
                }
                                
                for (int i = 0; i < SAMPLES_N; i++)
                {                   
                    SPSCQueueConflated<NoDefaultConstructible>::Proxy& item = *updates[i % 10];
                    item.data.i = 1;
                    q.push(item);
                }

                finished.store(true, std::memory_order_release);

            });

            thread c([&q, &updatesN, &finished]() {
                std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

                NoDefaultConstructible data(0);
                while (!finished.load(std::memory_order_acquire))
                {
                    if (!q.pop(data))
                        continue;                    
                    else
                    {
                        updatesN++;
                    }
                }

                std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                cout << "Took ->" << (double)std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / updatesN
                     << " microseconds per item" << std::endl;
            });

            c.join();
            p.join();
           
            cout << "\n moveN = " << moveN << "; moveAssingN = " << moveAssingN
                 << "; copyN = " << copyN << "; copyAssingN = " << copyAssingN;

            cout << "\nConflated -> " << conflatedN
                 << "; Recived -> " << updatesN;
        }
    }
}
