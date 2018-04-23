#pragma once

#include <thread>
#include <chrono>
#include "SPSCQueue.h"
#include "OrderBook.h"

namespace tests
{
    constexpr size_t N = 1000000;
    constexpr size_t Q_SIZE = 10;

    class WrapInt
    {
    public:
        WrapInt() = delete;
       // WrapInt(int input) { v = input;  }
        //WrapInt(const WrapInt& other) { v = other.v; }
       // void setValue(int input) { v = input; }
      //  int getValue() { return v;  }

        operator int() { return v;  }
       // operator int& () { return v; }
        operator int* () { return &v; }

        WrapInt& operator= (int other) { v = other; return *this; }
       // WrapInt& operator= (const WrapInt& other) { v = other.v; return *this; }

    private:
        int v;
    };

    void producerSPSC(reference_wrapper<SPSCQueue<WrapInt>>& q)
    {
        WrapInt* item = nullptr;
        for (int i = 0; i < N; i++)
        {            
            q.get().getForWrite(&item);
            {
                *item = 1;
            }
            q.get().notifyPush();
        }
    }

    void consumerSPSC(reference_wrapper<SPSCQueue<WrapInt>>& q)
    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        int count = 0;
        WrapInt* item = nullptr;
        for (int i = 0; i < N; i++)
        {
            q.get().getForRead(&item);
            {
                count += *item;
            }
            q.get().notifyPop();
        }

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        cout << "Took ->" << (double) std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / N
             << " microseconds per item" << std::endl;

        cout << "Consumer result -> " << count << endl;
    }

    void testSPSC()
    {
        {
            SPSCQueue<WrapInt> q(Q_SIZE);

            thread p(producerSPSC, ref(q));
            thread c(consumerSPSC, ref(q));

            p.join();
            c.join();
        }

        {
            SPSCQueue<OrderBook> q(Q_SIZE);
            constexpr static int LEN = 10;

            thread p([&q]() {
                OrderBook* item = nullptr;
                for (int i = 0; i < LEN; i++)
                {
                    q.getForWrite(&item);
                    {
                        item->price = i;
                        item->side = 'B';
                        item->volume = 2*i;

                        string instrument("GOOOOOOOOOOOOOOOOOOO00000000000000000000000000000000000000000000000000000000000000OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOGLE");
                        for (int j = 0; j < i % 20; j++)                        
                            instrument += (char)(65 + j);
                        
                        item->instrument = instrument;
                    }
                    q.notifyPush();
                }
            });

            thread c([&q]() {
                OrderBook* item = nullptr;
                for (int i = 0; i < LEN; i++)
                {
                    q.getForRead(&item);
                    {  
                        //cout << item->str();
                    }
                    q.notifyPop();
                }
            });

            p.join();
            c.join();
        }
    }
}
