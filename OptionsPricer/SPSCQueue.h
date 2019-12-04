#pragma once

#include <cstring>
#include <vector>
#include <atomic>

template <typename T>
class SPSCQueue
{
public:
    explicit SPSCQueue(size_t length)
        :N(length), readPos(0), writePos(0)
    {
        queue.resize(N);
        size_t tSize = sizeof(T);
    }

    // only called by producer
    void getForWrite(T** item, bool blocking = true)
    {
        while (1)
        {
            bool available = (queue[writePos].rxReady == 0);    
            atomic_thread_fence(std::memory_order_acquire);
            if (available)
            {
                
                *item = reinterpret_cast<T*>(&queue[writePos].raw);
                atomic_thread_fence(std::memory_order_acquire);
                break;
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
                    break;
                }
            }
        }
    }

    /* Producer must first call getForWrite in order
    * to retrieve a free item for editing and only
    * then call the notifyPush method
    */
    void notifyPush()
    {
        atomic_thread_fence(std::memory_order_release);
        queue[writePos].rxReady = 1;

        if (++writePos == N)
            writePos = 0;
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

        if (++readPos == N)
            readPos = 0;
    }
    
private:  
    struct Item
    {
        uint8_t rxReady = 0;
        char raw[sizeof(T)];
    };

    std::vector<Item> queue;    
    size_t N;

    alignas(64) size_t readPos;
    alignas(64) size_t writePos;    
};



