#pragma once

#include <cstring>
#include <vector>
#include <atomic>
#include <memory>

template <typename T>
class MPSCQueue
{
public:
    explicit MPSCQueue(size_t length)
        :N(length), readPos(1), writePos(0)
    {
        queue.resize(N); 

        int power = [length]() {
            if (length == 1)
                return 0;

            int power = 0;
            int devived = length;
            while (devived > 1)
            {
                devived = devived / 2;                
                power++;
            }

            if (devived == 1)
                return power;
            else
                return -1; //not power of 2!
        }();

        if (power == -1)
            throw std::runtime_error("lengh is not power of 2!");

        power++;
        clearMask = [power]() {
            size_t result = 1;
            for (int i = 0; i < power; i++)
                result |= 1 << i;
            return result;
        }();
    }

    // only called by producer
    size_t getForWrite(T** item, bool blocking = true)
    {
        size_t position = 0;
        while (1)
        {
            if (writePos.load(std::memory_order_acquire) > N * 10000)
                writePos.fetch_and(clearMask); // prevent writePos overflow!

            position = (++writePos % N);
            uint8_t expected = 1;
            if (queue[position].txReady->compare_exchange_strong(
                expected, 0, std::memory_order_acq_rel))
                break; // we got the slot!
        }

        while (1)
        {

            bool rxAvailable = (queue[position].rxReady == 0);
            atomic_thread_fence(std::memory_order_acquire);
            if (rxAvailable)
            {
                //queue[writePos].txReady = 0;
                *item = reinterpret_cast<T*>(&queue[position].raw);
                atomic_thread_fence(std::memory_order_acquire);
                return position;
            }
            else
            {
                if (blocking)
                {
                    _mm_pause();
                }
                else
                {
                    *item = 0;  
                    return position;
                }
            }
        }
    }

    /* Producer must first call getForWrite in order
    * to retrieve a free item for editing and only
    * then call the notifyPush method
    */
    void notifyPush(size_t position)
    {
        atomic_thread_fence(std::memory_order_release);
        queue[position].rxReady = 1;
    }

    // only called by consumer
    void getForRead(T** item, bool blocking = true)
    {
        while (1)
        {
            bool empty = (queue[readPos].rxReady == 0);
            atomic_thread_fence(std::memory_order_acquire);
            if (empty)
            {
                if (blocking)
                {
                    _mm_pause();
                }
                else
                { 
                    *item = 0;
                    break;
                }
            }
            else
            {
                *item = reinterpret_cast<T*>(&queue[readPos].raw);
                atomic_thread_fence(std::memory_order_acquire);
                break;
            }
        }
    }

    /* Consumer must first call getForWrite in order
    * to retrieve a free item for editing and only
    * then call the push method
    */

    void notifyPop()
    {
        atomic_thread_fence(std::memory_order_release);
        queue[readPos].rxReady = 0;   
        queue[readPos].txReady->store(1, std::memory_order_seq_cst);
        readPos = ++readPos % N;
    }

private:
    struct Item
    {
        uint8_t rxReady = 0;
        char raw[sizeof(T)];
        std::unique_ptr<std::atomic<uint8_t>> txReady = 
            std::make_unique<std::atomic<uint8_t>>(1);
    };

    size_t clearMask;
    std::vector<Item> queue;
    size_t N;

    alignas(64) size_t readPos;
    alignas(64) std::atomic<size_t> writePos;
};



