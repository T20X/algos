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
    struct ConflatedItemProxy;
    using Proxy = ConflatedItemProxy<T>;

    explicit SPSCQueueConflated(size_t length)
        :N(length), readPos(0), writePos(0)
    {
        queue.resize(N);       
    }

    bool push(Proxy& value)
    {
        if (value.notification &&
            value.notification->rxReady)
        {
            // conflated!
            return true;
        }

        atomic_thread_fence(std::memory_order_acquire);

        Notification* item = nullptr;
        if (!getForWrite(&item))
            return false; // q is full!

        value.notification = item;
        item->data = &value.data;            
     
        notifyPush();
        return true;
    }

    bool pop(T& value)
    {       
        if (!getForRead(value, false))
            return false;

        notifyPop();
        return true;
    }

    template <typename T>
    struct ConflatedNotification;

    template <typename T>
    struct ConflatedItemProxy
    {
        using Ptr = ConflatedItemProxy<T>*;

        union
        {
            char raw[sizeof(T)];
            T data;
        };

        ConflatedNotification<T>* notification = nullptr;

        //ConflatedItemProxy() = default;

        ConflatedItemProxy(const T& d, ConflatedNotification<T>* n)
            : data(d), notification(n) {}

        ConflatedItemProxy(T&& d, ConflatedNotification<T>* n)
            : data(std::move(d)), notification(n) {}

        ConflatedItemProxy(const ConflatedItemProxy& other) = delete;
        ConflatedItemProxy(ConflatedItemProxy&& other) = delete;

        ConflatedItemProxy& operator= (const ConflatedItemProxy& other) = delete;        
        ConflatedItemProxy& operator= (ConflatedItemProxy&& other) = delete;

        //~ConflatedItemProxy() {}

       /* ConflatedItemProxy(ConflatedItemProxy&& other)
        {
            data = std::move(other.data);
            notification = other.notification;
        }

        ConflatedItemProxy& operator= (ConflatedItemProxy&& other)
        {
            if (this != &other)
            {
                data = std::move(other.data);
                notification = other.notification;
            }

            return *this;
        }

        ConflatedItemProxy(const ConflatedItemProxy& other)
        {
            data = other.data;
            notification = other.notification;
        }

        ConflatedItemProxy& operator= (const ConflatedItemProxy& other)
        {
            if (this != &other)
            {
                data = other.data;
                notification = other.notification;
            }

            return *this;
        }*/
    };

private:
    using Notification = ConflatedNotification<T>;

    // only called by producer
    bool getForWrite(Notification** item, bool blocking = true)
    {
        while (1)
        {
            bool available = (queue[writePos].rxReady == 0);    
            atomic_thread_fence(std::memory_order_acquire);
            if (available)
            {                
                *item = &queue[writePos];
                return true;
            }
            else
            {
                if (blocking)                
                    _mm_pause();                
                else                
                    return false;                                    
            }
        }
    }

    /* Producer must first call getForWrite in order
    * to retrieve a free msg for editing and only
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
    bool getForRead(T& item, bool blocking = true)
    {
        while (1)
        {
            bool empty = (queue[readPos].rxReady == 0);   
            atomic_thread_fence(std::memory_order_acquire);
            if (empty)
            {
                if (blocking)                
                    _mm_pause();                
                else                     
                    return false;              
            }
            else
            {
                item = std::move(*queue[readPos].data);
                return true;
            }
        }
    }

    /* Consumer must first call getForWrite in order
    * to retrieve a free msg for editing and only
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
    template <typename T>
    struct ConflatedNotification
    {
        uint8_t rxReady = 0;
        T* data = nullptr;
    };

    std::vector<Notification> queue;
    size_t N;

    alignas(64) size_t readPos;
    alignas(64) size_t writePos;    
};


/*template <typename T>
inline bool operator== (const typename SPSCQueueConflated<T>::Proxy& left,
                        const typename SPSCQueueConflated<T>::Proxy& right)
{
    return (left.id == right.id);
}*/


