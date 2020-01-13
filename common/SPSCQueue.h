#pragma once

#include <new>
#include <atomic>
#include <array>
#include <type_traits>

#if LINUX > 0
#define _relax_ __asm__ __volatile("pause" ::: "memory")
#else
#include <emmintrin.h>
#define _relax_ _mm_pause()
#endif


template <class T, bool blocking=true>
class SPSCQueue {
public:
    struct Node {        
        std::atomic<bool> busy{0};
        alignas(alignof(T)) std::byte raw[sizeof(T)];
    };


    using node_ptr = Node*;
    using node_ref = Node&;    
    using type_ptr = T*;

    explicit SPSCQueue(uint32_t N)
         :d_(static_cast<node_ptr>(operator new(sizeof(Node)*N,
               std::align_val_t(alignof(Node))))),         
         len_(N) {
        for (uint32_t i = 0; i < len_; ++i)
            new (static_cast<void*>(d_ + i)) Node{};
    }

    bool add(const T& o) {
        uint32_t pos = writePos_++ % len_;
        node_ref item = *(d_+pos);
        do
        {
            if (!item.busy.load(std::memory_order_acquire))
            {
                new (static_cast<void*>(item.raw)) T(o);
                item.busy.store(1, std::memory_order_release);
                return true;
            }

            _relax_;
        } while (blocking);
        return false;
    }

    template <class... Args>
    bool emplace(Args&&... params) {
        uint32_t pos = writePos_++ % len_;
        node_ref item = *(d_ + pos);
        do
        {
            if (!item.busy.load(std::memory_order_acquire))
            {
                new (static_cast<void*>(item.raw)) T(std::forward<Args>(params)...);
                item.busy.store(1, std::memory_order_release);
                return true;
            }

            _relax_;
        } while (blocking);
        return false;
    }

    T extract() { 
        uint32_t pos = readPos_++ % len_;
        node_ref item = *(d_+pos);
        while (!item.busy.load(std::memory_order_acquire)) {     
            _relax_;
        } 

        T& val = *reinterpret_cast<type_ptr>(item.raw);
        T tmp(std::move(val));
        val.~T();

        item.busy.store(0, std::memory_order_release);
        return tmp;
    }

    template <class F>
    bool visit(const F& f, bool block = true) {
        uint32_t pos = readPos_++ % len_;
        Node& item = *(d_+pos);
        do {           
            if (item.busy.load(std::memory_order_acquire))
            {
                T& val = *reinterpret_cast<type_ptr>(item.raw);
                f(std::move(val));
                val.~T();

                item.busy.store(0, std::memory_order_release);
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
        for (uint32_t i = 0; i < len_; ++i)   {
            Node& item = *(d_ + i);
            if (item.busy.load(std::memory_order_acquire)) {
                T& val = *reinterpret_cast<type_ptr>(item.raw); 
                val.~T();

                item.busy.store(0, std::memory_order_release);
            }
        }

        operator delete (d_, std::align_val_t(alignof(Node)));
    }

private:
    SPSCQueue(const SPSCQueue&) = delete;
    SPSCQueue& operator=(const SPSCQueue&) = delete;

    alignas(64) uint32_t readPos_ { 0 };
    alignas(64) uint32_t writePos_ { 0 };
    alignas(64) node_ptr d_;
    uint32_t len_;
};

template <class T, bool blocking>
struct SPSCQueueFixedBase
{ 
    using type_ptr = T*;
    using Storage = std::aligned_storage_t<
        sizeof(T), alignof(T)>;

    struct Node {
        bool busy{ 0 };
        Storage raw;
        std::atomic<bool>& lock() {
            return *reinterpret_cast<std::atomic<bool>*>(
                static_cast<void*>(&this->busy));
        }
    }; 

    SPSCQueueFixedBase() = default;
    bool add_(const T& o, Storage& s, std::atomic<bool>& lock) {              
        do {
            if (!lock.load(std::memory_order_acquire)) {
                new (static_cast<void*>(&s)) T(o);
                lock.store(1, std::memory_order_acquire);
                return true;
            }

            _relax_;
        } while (blocking);
        return false;
    }

    template <class... Args>
    bool emplace_(Storage& s, std::atomic<bool>& lock, Args&&... params) {
        do {
            if (!lock.load(std::memory_order_acquire)) {
                 new (static_cast<void*>(&s)) T(std::forward<Args>(params)...);
                 lock.store(1, std::memory_order_release);
                return true;
            }

            _relax_;
        } while (blocking);
        return false;
    }

    void destroy(Storage& s, std::atomic<bool>& lock) {
        if (lock.load(std::memory_order_acquire)) {
            T& val = *reinterpret_cast<type_ptr>(&s);
            val.~T();
        }
    }

    T extract_(Storage& s, std::atomic<bool>& lock) {
        while (!lock.load(std::memory_order_acquire)) {
            _relax_;
        }

        T& val = *reinterpret_cast<type_ptr>(&s);
        T tmp(std::move(val));
        val.~T();

        lock.store(0, std::memory_order_release);
        return tmp;
    }
};

template <class T, uint32_t N, bool blocking=true>
class SPSCQueueFixed : private SPSCQueueFixedBase<T,blocking> {
private:   
    using Base = SPSCQueueFixedBase<T, blocking>;
    using typename Base::Node;

    using node_ptr = Node*;
    using node_ref = Node&;

    alignas(64) std::array<Node, N> d_;
    alignas(64) uint32_t readPos_ { 0 };
    alignas(64) uint32_t writePos_ { 0 };

    SPSCQueueFixed(const SPSCQueueFixed&) = delete;
    SPSCQueueFixed& operator=(const SPSCQueueFixed&) = delete;
    static_assert((N & (N - 1)) == 0, "N must be power of 2!");

public:
    SPSCQueueFixed() = default;
    ~SPSCQueueFixed() {
        if (!std::is_trivially_destructible<T>::value) {
            for (uint32_t i = 0; i < N; ++i) {
                Node& item = d_[i];
                Base::destroy(item.raw, item.lock());
            }
        }
    }

    bool add(const T& o) {
        uint32_t pos = writePos_++; pos &=(N-1);
        node_ref& item = d_[pos];
        return Base::add_(o, item.raw, item.lock());
    }

    template <class... Args>
    bool emplace (Args&&... params) {
        uint32_t pos = writePos_++ % N;
        node_ref& item = d_[pos];
        return Base::emplace_(item.raw, item.lock(), std::forward<Args>(params)...);
    }

    T extract() {
        uint32_t pos = readPos_++ % N;
        node_ref& item = d_[pos];
        return Base::extract_(item.raw, item.lock());
    }
};


template <class T, uint32_t N, bool blocking = true>
class SPSCQueueFixed2 {
private: 
    alignas(64) uint32_t readPos_ { 0 };
    alignas(64) uint32_t writePos_ { 0 };    
    std::aligned_storage_t<sizeof(T), alignof(T)> raw[N];
    alignas(64) std::array<bool, N> s_ = { false };

    std::atomic<bool>& lock(uint32_t pos) {
        return *reinterpret_cast<std::atomic<bool>*>(
            static_cast<void*>(&s_[pos]));
    }

    SPSCQueueFixed2(const SPSCQueueFixed2&) = delete;
    SPSCQueueFixed2& operator=(const SPSCQueueFixed2&) = delete;

public:
    SPSCQueueFixed2() = default;
    ~SPSCQueueFixed2() {
        for (uint32_t i = 0; i < N; ++i) {            
            if (lock(i).load(std::memory_order_acquire)) {
                T& val = reinterpret_cast<T&>(raw[i]);
                val.~T();

                lock(i).store(0, std::memory_order_release);
            }
        }
    }

    bool add(const T& o) {
        uint32_t pos = writePos_++ % N;
        auto& l = lock(pos);
        do {
            if (!l.load(std::memory_order_acquire)) {
                new (static_cast<void*>(&raw[pos])) T(o);
                l.store(1, std::memory_order_release);
                return true;
            }

            _relax_;
        } while (blocking);
        return false;
    }

    template <class... Args>
    bool emplace(Args&&... params) {
        uint32_t pos = writePos_++ % N;
        auto& l = lock(pos);
        do {
            if (!l.load(std::memory_order_acquire)) {
                new (static_cast<void*>(&raw[pos])) T(std::forward<Args>(params)...);
                l.store(1, std::memory_order_release);
                return true;
            }

            _relax_;
        } while (blocking);
        return false;
    }

    T extract() {  
        uint32_t pos = readPos_++ % N;       
        auto& l = lock(pos);
        while (!l.load(std::memory_order_acquire)) {
            _relax_;
        }

        T& val = reinterpret_cast<T&>(raw[pos]);
        T tmp(std::move(val));
        val.~T();

        l.store(0, std::memory_order_release);
        return tmp;
    }
};

