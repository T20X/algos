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
        constexpr static size_t SAMPLES_N = 10000000;

        {
            int conflatedN = 0;
            int updatesN = 0;
        
            atomic<bool> finished(false);

            SPSCQueueConflated<int> q(2);

            thread p([&q, &conflatedN, &finished]() {
                /*unordered_set<SPSCQueueConflated<int>::ConflatedType> updates;
                for (size_t i = 0; i < 10; i++)
                {
                    ConflatedItem<int> d1; d1.id = i;
                    updates.emplace(d1);
                }*/

                /*ConflatedItem<int> data;
                ConflatedNotification<int>* item = nullptr;

                for (int i = 0; i < SAMPLES_N; i++)
                {
                    if (data.notification && 
                        data.notification->busy)
                    {
                        conflatedN++;
                        continue;
                    }

                    atomic_thread_fence(std::memory_order_release);
                    q.getForWrite(&item);
                    {
                        data.notification = item;
                        item->data = &data;
                        item->data->data = 1;                        
                    }
                    q.notifyPush();
                }*/

                //ConflatedNotification<int>* item = nullptr;
                SPSCQueueConflated<int>::ConflatedType data; data.id = 1;
                for (int i = 0; i < SAMPLES_N; i++)
                {                   
                    data.data = i;
                    q.push(&data);
                }

                finished.store(true, std::memory_order_release);

            });

            thread c([&q, &updatesN, &finished]() {
                std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

                SPSCQueueConflated<int>::ConflatedType* data = nullptr;
                while (!finished.load(std::memory_order_acquire))
                {
                    if (!q.pop(&data)) continue;                    
                    {
                        updatesN++;
                    }
                }

               /* ConflatedItem<int>* data = nullptr;
                ConflatedNotification<int>* item = nullptr;
                while (!finished.load(std::memory_order_acquire))
                {
                    q.getForRead(&item, false);
                    if (!item) continue;
                    {
                        updatesN++;
                    }
                    q.notifyPop();
                }*/

                std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                cout << "Took ->" << (double)std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / updatesN
                     << " microseconds per item" << std::endl;
            });

            c.join();
            p.join();
           

            cout << "\nConflated -> " << conflatedN
                 << "; Recived -> " << updatesN;
        }
    }
}
