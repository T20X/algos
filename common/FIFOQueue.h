#pragma once


#include <deque>
#include <tuple>
#include <string>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <list>
#include <thread>
#include "basics.h"


template <typename ItemType, typename Queue = std::deque<ItemType>>
class FIFOQueue
{
public:
    FIFOQueue() {}

    template <typename Value>
    void add(Value&& v)
    {
        unique_lock<mutex> lock(m_guard);
        m_queue.emplace_back(forward<Value>(v));
        lock.unlock();

        m_goodNews.notify_one();
    }

    void stop()
    {
        unique_lock<mutex> lock(m_guard);
        m_stop = true;
        lock.unlock();

        m_goodNews.notify_all();
    }

    bool pop(ItemType& item, bool isBlocking = true)
    {
        std::unique_lock<mutex> lock(m_guard);
        if (m_stop) return false;

        do
        {
            if (!m_queue.empty())
            {
                item = m_queue.front();
                m_queue.pop_front();
                return true;
            }
            else if (isBlocking)
            {
                m_goodNews.wait(lock);                
            }
            else
            {
                return false;
            }
        }
        while (!m_stop);
        return false;
    }

    void shrink()
    {
        std::unique_lock<mutex> lock(m_guard);
        m_queue.shrink_to_fit();
    }

    bool empty()
    {
        std::unique_lock<mutex> lock(m_guard);
        return m_queue.empty();
    }

    bool isStopped() 
    {
        unique_lock<mutex> lock(m_guard);
        return m_stop;
    }

    FIFOQueue(const FIFOQueue&) = delete;
    FIFOQueue(FIFOQueue&&) = delete;

    FIFOQueue& operator=(const FIFOQueue&) = delete;
    FIFOQueue& operator=(FIFOQueue&&) = delete;

private:
    Queue m_queue;
    std::mutex m_guard;
    std::condition_variable m_goodNews;
    bool m_stop = false;
};

void testFIFOQueue()
{
    struct Order
    {
        int qty;
        double px;
        uint64_t id;
    };

    std::atomic<uint32_t> _counter_(1);
    std::atomic<uint32_t> stop(0);
    FIFOQueue<Order> q;

    auto consume = [&_counter_, &stop](int id, FIFOQueue<Order>& q) {
        basics::print(id, "therad started");

        while (!stop.load() && !q.isStopped())
        {
            Order o;
            if (q.pop(o))
            {
                --_counter_;

                if (!_counter_)
                {
                    stop.store(1);
                    q.stop();
                }
            }
        }
    };


   
    std::list<std::thread> l;
    size_t N = 10000;
    for (int i = 1; i < 5; i++) 
        l.push_back(std::thread(consume, i, std::ref(q)));

 

    auto publish = [&](int id) {
        basics::print("[", id, "] ", "therad started");

        while (N)
        {
            ++_counter_;
            q.add(Order{ 1,12.2,N });
            N--;
        }

        Order o{ 0,0,0 };
        q.add(o); // trigger stop!
    };

    std::thread p(publish, 0);
    p.join();

    for (std::thread& t : l)
        t.join();
}