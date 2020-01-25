#ifndef _SPSC_BOUNDED_QUEUE_HPP
#define _SPSC_BOUNDED_QUEUE_HPP

#include <cstring>
#include <vector>

#include "Config.hpp"
#include "Common.hpp"

template <typename T>
class SPSCBoundedQueue
{
public:
    explicit SPSCBoundedQueue(size_t N)
        : length(N)
    {
        queue.resize(N);
        readPos = writePos = 0;
        memset(&queue[0], 0,  length * sizeof(T));
    }

    // only called by producer
    void getForWrite(T** item, bool blocking = true)
    {
        while(1)
        {
            bool available = (queue[writePos].busy == 0); 
            full_compiler_fence();
            if (likely(available))
            {
                *item = const_cast<T*>(&queue[writePos]);   
                full_compiler_fence();
                break;
            }
            else
            {
                if (blocking)
                {
                    cpuPause();
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
        full_compiler_fence();
        queue[writePos].busy = 1;

        if (++writePos == length)
            writePos = 0;
    }

    // only called by consumer
    void getForRead(T** item, bool blocking = true)
    {
        while (1) 
        {
            bool empty = (queue[readPos].busy == 0); 
            full_compiler_fence();
            if (unlikely(empty))
            {
                if (blocking)
                { 
                    cpuPause();
                }
                else
                {
                    *item = 0;
                    break;
                }
            }
            else
            {
                *item = const_cast<T*>(&queue[readPos]);
                full_compiler_fence();
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
        full_compiler_fence();
        queue[readPos].busy = 0;

        if (++readPos == length)
            readPos = 0;
    }

private:
    std::vector<T> queue;
    size_t length;

    char padding1[CACHE_LINE_SIZE];
    volatile size_t readPos; 
    char padding2[CACHE_LINE_SIZE-sizeof(size_t)];
    volatile size_t writePos;
    char padding3[CACHE_LINE_SIZE-sizeof(size_t)];
};

#endif // _SPSC_BOUNDED_QUEUE_HPP

