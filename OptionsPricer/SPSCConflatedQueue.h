#pragma once
#pragma once

#include <cstring>
#include <vector>
#include <atomic>

template <typename T>
class SPSCQueueConflated
{
public:
    template <typename T>
    struct ConflatedNotification;

    template <typename T>
    struct ConflatedItem
    {
        T data;
        uint64_t id = 0;
        ConflatedNotification<T>* notification = nullptr;
    };

    template <typename T>
    struct ConflatedNotification
    {
        uint8_t busy = 0;
        ConflatedItem<T>* data = nullptr;
    };

    using Type = ConflatedNotification<T>;
    using ConflatedType = ConflatedItem<T>;

    explicit SPSCQueueConflated(size_t length)
        :N(length), readPos(0), writePos(0)
    {
        queue.resize(N);       
    }

    bool push(ConflatedType* value)
    {
        if (value->notification &&
            value->notification->busy)
        {
            // conflated!
            return true;
        }

        atomic_thread_fence(std::memory_order_acquire);
        Type* item = nullptr;
        getForWrite(&item);

        if (!item) return false; // q is full!
        value->notification = item;
        item->data = value;            
     
        notifyPush();
        return true;
    }

    bool pop(ConflatedType** value)
    {
        Type* item = nullptr;
        getForRead(&item, false);

        if (!item)
        {
            *value = nullptr;
            return false;
        }

        *value = item->data;
        
        notifyPop();
        return true;
    }

private:

    // only called by producer
    void getForWrite(Type** item, bool blocking = true)
    {
        while (1)
        {
            bool available = (queue[writePos].busy == 0);    
            atomic_thread_fence(std::memory_order_acquire);
            if (available)
            {
                
                *item = reinterpret_cast<Type*>(&queue[writePos]);
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
        queue[writePos].busy = 1;

        if (++writePos == N)
            writePos = 0;
    }

    // only called by consumer
    bool getForRead(Type** item, bool blocking = true)
    {
        while (1)
        {
            bool empty = (queue[readPos].busy == 0);   
            atomic_thread_fence(std::memory_order_acquire);
            if (empty)
            {
                if (blocking)
                {
                    _mm_pause();
                }
                else
                {
                    *item = nullptr;
                    return false;
                }
            }
            else
            {
                *item = reinterpret_cast<Type*>(&queue[readPos]);
                return true;
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
        queue[readPos].busy = 0;

        if (++readPos == N)
            readPos = 0;
    }
    
private:  
    std::vector<Type> queue;
    size_t N;

    alignas(64) size_t readPos;
    alignas(64) size_t writePos;    
};


template <typename T>
inline bool operator== (const typename SPSCQueueConflated<T>::ConflatedType& left,
                        const typename SPSCQueueConflated<T>::ConflatedType& right)
{
    return (left.id == right.id);
}


