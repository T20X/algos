#pragma once

#include <new>
#include <atomic>

#if LINUX > 0
#define _relax_ __asm__ __volatile("pause" ::: "memory")
#else
#include <emmintrin.h>
#define _relax_ _mm_pause()
#endif


template <class T>
class SPSCQueue {
public:
    struct Node {        
        alignas(alignof(T)) std::byte raw[sizeof(T)];
        std::atomic<std::byte> busy{std::byte(0)};
    };

    explicit SPSCQueue(size_t N)
         :d_(static_cast<Node*>(::operator new(sizeof(Node)*N, 
               std::align_val_t(alignof(Node))))),         
         len_(N) {
        for (size_t i = 0; i < len_; ++i)
            new (static_cast<void*>(d_ + i)) Node{};
    }

    bool add(const T& o, bool block=true) {
        size_t pos = writePos_++ % len_;
        Node& item = *(d_+pos);
        do
        {
            if (!std::to_integer<uint8_t>(item.busy.load(
                   std::memory_order_acquire)))
            {
                new (static_cast<void*>(item.raw)) T(o);
                item.busy.store(std::byte(1), std::memory_order_release);
                return true;
            }

            _relax_;
        } while (block);
        return false;
    }

    T extract() { 
        size_t pos = readPos_++ % len_;
        Node& item = *(d_+pos);
        while (!std::to_integer<uint8_t>(item.busy.load(
                  std::memory_order_acquire)))
        {     
            _relax_;
        } 

        T tmp(*reinterpret_cast<T*>(item.raw));
        item.busy.store(std::byte(0), std::memory_order_release);
        return tmp;
    }

    template <class F>
    bool visit(const F& f, bool block = true) {
        size_t pos = readPos_++ % len_;
        Node& item = *(d_+pos);
        do
        {           
            if (std::to_integer<uint8_t>(item.busy.load(
                  std::memory_order_acquire)))
            {
                f(*reinterpret_cast<T*>(item.raw));
                item.busy.store(std::byte(0), std::memory_order_release);
                return true;
            }

            _relax_;
        } while (block);
        return false;
    }

    bool extract(T& o, bool block=true) {
        visit([&o](auto& v){ o = v; }, block);
    }

    ~SPSCQueue() {       
       ::operator delete (d_, std::align_val_t(alignof(Node)));
    }

private:
    alignas(64) size_t readPos_;
    alignas(64) size_t writePos_;
    alignas(64) Node* d_;
    size_t len_;
};
