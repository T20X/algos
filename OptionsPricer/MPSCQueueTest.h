#pragma once

#include <thread>
#include <chrono>
#include "MPSCQueue.h"
#include "OrderBook.h"

namespace tests
{
    constexpr size_t MPSC_THREADS = 10;
    constexpr size_t N_MPSC = 5000000;
    constexpr size_t Q_MPSC_SIZE = 30;

    class WrapInt2
    {
    public:
        WrapInt2() = delete;
        // WrapInt2(int input) { v = input;  }
        //WrapInt2(const WrapInt2& other) { v = other.v; }
        // void setValue(int input) { v = input; }
        //  int getValue() { return v;  }

        operator int() { return v; }
        // operator int& () { return v; }
        operator int* () { return &v; }

        WrapInt2& operator= (int other) { v = other; return *this; }
        // WrapInt2& operator= (const WrapInt2& other) { v = other.v; return *this; }

    private:
        int v;
    };

    void producerMPSC(reference_wrapper<MPSCQueue<WrapInt2>>& q)
    {
        WrapInt2* item = nullptr;
        for (int i = 0; i < N_MPSC; i++)
        {
            size_t position = q.get().getForWrite(&item);
            {
                *item = 1;
            }
            q.get().notifyPush(position);
        }
    }

    void consumerMPSC(reference_wrapper<MPSCQueue<WrapInt2>>& q)
    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        int count = 0;
        WrapInt2* item = nullptr;
        for (int i = 0; i < N_MPSC * MPSC_THREADS; i++)
        {
            q.get().getForRead(&item);
            {
                count += *item;
            }
            q.get().notifyPop();
        }

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        cout << "Took ->" << (double)std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / N_MPSC
            << " microseconds per item" << std::endl;

        cout << "Consumer result -> " << count << endl;
    }

    void testMPSC()
    {
        {
            MPSCQueue<WrapInt2> q(Q_MPSC_SIZE);

            vector<thread> pool;
            for (int i = 0; i < MPSC_THREADS; i++)
                pool.emplace_back(producerMPSC, ref(q));

            thread c(consumerMPSC, ref(q));

            for (auto& t : pool)
                t.join();

            c.join();
        }

        {
            MPSCQueue<OrderBook> q(Q_MPSC_SIZE);
            constexpr static int LEN = 10;

            thread p([&q]() {
                OrderBook* item = nullptr;
                for (int i = 0; i < LEN; i++)
                {
                    size_t position = q.getForWrite(&item);
                    {
                        item->price = i;
                        item->side = 'B';
                        item->volume = 2 * i;

                        string instrument("GOOOOOOOOOOOOOOOOOOO00000000000000000000000000000000000000000000000000000000000000OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOGLE");
                        for (int j = 0; j < i % 20; j++)
                            instrument += (char)(65 + j);

                        item->instrument = instrument;
                    }
                    q.notifyPush(position);
                }
            });

            thread c([&q]() {
                OrderBook* item = nullptr;
                for (int i = 0; i < LEN; i++)
                {
                    q.getForRead(&item);
                    {
                        cout << item->str();
                    }
                    q.notifyPop();
                }
            });

            p.join();
            c.join();
        }
    }
}

