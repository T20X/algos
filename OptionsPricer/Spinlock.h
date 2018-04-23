#pragma once

#include <atomic>

class ScopedSpinlock2
{
public:
    ScopedSpinlock2(std::atomic<bool>& g)
        :guard(g), locked(false)
    {
        lock();
    }

    ~ScopedSpinlock2()
    {
        unlock();
    }

    void lock()
    {
        bool expected = false;
        while (!guard.compare_exchange_weak(expected, true, std::memory_order_acq_rel))
        {   
             expected = false;
            _mm_pause();
        }

        //got it!
        locked = true;        
    }

    void unlock()
    {
        if (locked)
        {
            guard.store(false, std::memory_order_release);
            locked = false;
        }
    }

private:
    std::atomic<bool>& guard;
    bool locked;
};

class ScopedSpinlock
{
public:
    ScopedSpinlock(std::atomic_flag& g)
        :guard(g), locked(false)
    {
        lock();
    }

    ~ScopedSpinlock()
    {
        unlock();
    }

    void lock()
    {        
        while (guard.test_and_set(std::memory_order_acq_rel))
            _mm_pause();
     
        //got it!
        locked = true;
    }

    void unlock()
    {
        if (locked)
        {
            guard.clear(std::memory_order_release);
            locked = false;
        }
    }

private:
    std::atomic_flag& guard;
    bool locked;
};

