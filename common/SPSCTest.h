#pragma once

#include "SPSCQueue.h"
#include <thread>
#include <chrono>
#include <functional>
#include <string>
#include <sstream>

namespace spsc
{
        using namespace std;
        constexpr size_t _N_ = 1000000;
        constexpr size_t Q_SIZE = 10;

        class WrapInt
        {
        public:
            //WrapInt() = delete;
            // WrapInt(int input) { v = input;  }
             //WrapInt(const WrapInt& other) { v = other.v; }
            // void setValue(int input) { v = input; }
           //  int getValue() { return v;  }

            operator int() { return v; }
            // operator int& () { return v; }
            operator int* () { return &v; }

            WrapInt& operator= (int other) { v = other; return *this; }
            // WrapInt& operator= (const WrapInt& other) { v = other.v; return *this; }

        private:
            int v;
        };

        struct OrderBook
        {
            int volume;
            double price;
            char side;
            string instrument;

            string str()
            {
                ostringstream o;
                o << "\n instrument: " << instrument << "; "
                    << side << "; " << price << "@" << volume;
                return o.str();
            }
        };

        void producerSPSC(reference_wrapper<SPSCQueue<WrapInt>> q)
        {
            WrapInt item;
            for (int i = 0; i < _N_; i++)
            {

                item = 1;
                q.get().add(item);                
            }
        }

        void consumerSPSC(reference_wrapper<SPSCQueue<WrapInt>> q)
        {
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

            int count = 0;
            //WrapInt* item = nullptr;
            for (int i = 0; i < _N_; i++)
            {
                WrapInt item = q.get().extract();
                count += item;               
            }

            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            cout << "Took ->" << (double)std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / _N_
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

            /*{
                SPSCQueue<OrderBook> q(Q_SIZE);
                constexpr static int LEN = 10;

                thread p([&q]() {
                    //OrderBook* item = nullptr;
                    OrderBook item;
                    for (int i = 0; i < LEN; i++)
                    {
                        
                        //q.getForWrite(&item);
                        {
                            item.price = i;
                            item.side = 'B';
                            item.volume = 2 * i;

                            string instrument("GOOOOOOOOOOOOOOOOOOO00000000000000000000000000000000000000000000000000000000000000OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOGLE");
                            for (int j = 0; j < i % 20; j++)
                                instrument += (char)(65 + j);

                            item.instrument = instrument;
                            q.add(item);
                        }
                        //q.notifyPush();
                    }
               });

                thread c([&q]() {
                    OrderBook item;
                    for (int i = 0; i < LEN; i++)
                    {
                        item = q.extract();                        
                    }
                });

                p.join();
                c.join();
            }*/
        }
    


    static void test()
    {
        testSPSC();

    }
}
