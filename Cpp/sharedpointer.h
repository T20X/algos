#pragma once

#include <atomic>
#include <boost/type_index.hpp>
#include "../common/basics.h"
#include "../common/config.h"
#include <memory>
#include <thread>
#include <list>
#include "../common/FIFOQueue.h"
#include <tuple>

#define _TRACE_SHARED_PTR_ 0

using namespace boost;
using namespace typeindex;

struct _shared_ptr_base
{
    using type = long;
    struct ControlBlock
    {        
        std::atomic<type> counter { 1 };
        std::atomic<type> counterWeak { 1 };
    };
};

template <typename T,     
          bool trace = false,
          std::enable_if_t<!std::is_array_v<typename T>, int> = 1>
class shared_pointer
{
public:
    using p_type = T;
    using p_ref = std::decay_t<T>&;
    using p_ptr = std::decay_t<T>*;
    using p_const_ref = const std::decay_t<T>&;
    using p_const_ptr = const std::decay_t<T>*;

    explicit shared_pointer(T* p = nullptr)
    {           
        bootsrap(p);
        #if _TRACE_SHARED_PTR_ == 1
            if (trace)
            {
                debug("constructor");           
            }
        #endif
    }

    shared_pointer(const shared_pointer& other) noexcept
    {
        #if _TRACE_SHARED_PTR_ == 1
            if (trace)
            {
               debug("copy constructor");
               other.debug("copy constructor other");
            }
        #endif
        
        acquire<shared_pointer, T>(other);
    }

    template <typename U>
    shared_pointer(const shared_pointer<U, trace>& other) noexcept
    {
        #if _TRACE_SHARED_PTR_ == 1
            if (trace)
            {
                debug("copy constructor <tempalted>");
                other.debug("copy constructor other <tempalted>");
            }
        #endif

        acquire<shared_pointer<U, trace>, U>(other);
    }

    shared_pointer(shared_pointer&& other) noexcept
    {
        #if _TRACE_SHARED_PTR_ == 1
            if (trace)
            {
                debug("move constructor");
                other.debug("move constructor other");
            }
        #endif

        shared_pointer(other).swap<T>((shared_pointer<T, trace>&)*this);
        //move_construct<shared_pointer,T>(std::move(other));
    }  

    template <typename U>
    shared_pointer(shared_pointer<U, trace>&& other) noexcept
    {
        #if _TRACE_SHARED_PTR_ == 1
            if (trace)
            {
                debug("move constructor <tempalted>");
                other.debug("move constructor other <tempalted>");
            }
        #endif

        shared_pointer(other).swap<T>(*this);
        //move_construct<shared_pointer<U, trace>, U>(std::move(other));
    }

    ~shared_pointer()
    {
        release();
    }

    template <typename U>
    shared_pointer& operator= (const shared_pointer<U, trace>& other) noexcept
    {
        #if _TRACE_SHARED_PTR_ == 1
             if (trace)
             {
                debug("assing operator <tempalated>");
                other.debug("ssing operator other <templated>");
             }
        #endif

        shared_pointer(other).swap(*this);
        return (*this);
        //return assing<shared_pointer<U, trace>, U>(other);
    }
    
    shared_pointer& operator= (const shared_pointer& other) noexcept
    {
        #if _TRACE_SHARED_PTR_ == 1
            if (trace)
            {
                debug("assing operator");
                other.debug("ssing operator other");
            }
        #endif

        shared_pointer(other).swap<T>((shared_pointer<T,trace>&)(*this));
        return (*this);
       // return assing<shared_pointer, T>(other);
    }

    shared_pointer& operator= (shared_pointer&& other)
    {
        #if _TRACE_SHARED_PTR_ == 1
            if (trace)
            {
                debug("move assing operator ");
                other.debug("move ssing operator other");
            }
        #endif

        shared_pointer(other).swap<T>(*this);
        return (*this);
        //return move_assign<shared_pointer, T>(other);
    }
    
    template <typename U>
    shared_pointer& operator= (shared_pointer<U, trace>&& other)
    {
        #if _TRACE_SHARED_PTR_ == 1
            if (trace)
            {
                debug("move assing operator <templated>");
                other.debug("move ssing operator other ");
            }
        #endif

        shared_pointer(other).swap(*this);
        return (*this);
    }

    T& operator* () noexcept
    {
        return *_p;
    }
    const T& operator* () const noexcept
    {
        return *_p;
    }

    T* get() noexcept
    {
        return _p; 
    }

    const T* get() const noexcept
    {
        return _p;
    }

    T* operator-> () noexcept
    {
        return _p; 
    }

    const T* operator-> () const noexcept 
    {
        return _p; 
    }

    operator bool() const noexcept { return _p; }

    bool operator== (const shared_pointer& other) const noexcept
    {
        return eq(other);
    }

    template <typename U>
    bool operator== (const shared_pointer<U, trace>& other) const noexcept
    {
        return eq(other);;
    }
    
    bool operator!= (const shared_pointer& other) const noexcept
    {
        return !operator==(other);
    }

    template <typename U>
    bool operator!= (const shared_pointer<U, trace>& other) const noexcept
    {
        return !operator==(other);
    }

    void reset(T* p = nullptr)
    {
        release();
        bootsrap(p);
    }

    long use_count() const noexcept
    {
        return _cb ? _cb->counter.load(std::memory_order_relaxed) : 0;
    }

    void metainfo()
    {
        cout << "p type -> " << basics::typeName<p_type>() << ", "
            << "p ptr -> " << basics::typeName<p_ptr>() << ", "
            << "p ref -> " << basics::typeName<p_ref>() << ", "
            << "p const_ref -> " << basics::typeName<p_const_ref>() << ", "
            << "p const_ptr -> " << basics::typeName<p_const_ptr>()
            << endl;
    }

    template <typename From, typename To>
    using allow_if_convertible = std::enable_if_t <std::is_convertible_v<
        std::add_pointer_t<From>, std::add_pointer_t<To>>, int>;

    template <typename U,
        typename = allow_if_convertible<std::add_pointer_t<U>, std::add_pointer_t<T>>,
        typename = allow_if_convertible<std::add_pointer_t<T>, std::add_pointer_t<U>>>
    void swap(shared_pointer<U, trace>& other) noexcept
    {
        std::swap(other._p, _p);
        std::swap(other._cb, _cb);
    }

private:
    friend class shared_pointer;

    void bootsrap(T* p)
    {
        _p = p;
         _cb = _p ? new _shared_ptr_base::ControlBlock : nullptr;            
    }

    template <typename U, typename UType>
    void clone(const U& other) noexcept
    {
         _p = other._p;       
        _cb = other._cb;
    }

    template <typename U, typename UType, typename = allow_if_convertible<UType,T>>
    void acquire(const U& other) noexcept
    {
        clone<U, UType>(other);
        if (_cb) _cb->counter.fetch_add(1, std::memory_order_relaxed);
    }

    void release()
    {
        if (_cb)
        {            
            if (_cb->counter.fetch_sub(1, std::memory_order_release) == 1)
            {
                std::atomic_thread_fence(std::memory_order_acquire);
                basics::del(_cb);
                basics::del(_p);
            }
        }
    }

private:

    template <typename U>
    bool eq(const shared_pointer<U, trace>& other) const noexcept
    {
        return _p == other._p && _cb == other._cb;
    }

    void cleanup() noexcept
    {
        _p = nullptr;
        _cb = nullptr;
    }

    void debug(const char* str) const noexcept
    {
        basics::print(str, "_p", _p, " _cb", _cb, "count", use_count());
    }

private:
     T * _p = nullptr;
    _shared_ptr_base::ControlBlock* _cb = nullptr;
};

namespace sharedtest
{
      
    struct CopyTest
    {
    public:
      
    };

    void copySharedPtr(shared_pointer<CopyTest, true>& o)
    {
        shared_pointer<CopyTest, true> p(new CopyTest());
        shared_pointer<CopyTest, true>& p1 = p;
        o = p1;
    }

    void testSharedPtrMain()
    {
        // CopyTest cp;
         //copyTest(cp);

        //
        {
            shared_pointer<CopyTest, true> ptr;
            shared_pointer<CopyTest, true> ptr1(new CopyTest);
            shared_pointer<CopyTest, true> ptr2(ptr1);
            copySharedPtr(ptr);

            if (ptr2 != ptr1)
            {
                basics::print(__func__, "BAD: Failed!", __LINE__);
                return;
            }
        }

        //
        {
            struct A
            {
            };

            struct B : public A
            {
            };

            shared_pointer<A, true> ptr1(new A);
           // shared_pointer<B, true> ptr2(ptr1);
            
            /*if (ptr2 != ptr1)
            {
                basics::print(__func__, "BAD: Failed!", __LINE__);
                return;
            }*/
        }

        //
        {
            struct A
            {
            };

            struct B : public A
            {
            };

            std::shared_ptr<A> ptr1_s(new A);
            std::shared_ptr<B> ptr2_s(new B);
          
            shared_pointer<A, true> ptr1(new A);
            shared_pointer<B, true> ptr2;
            //ptr2 = ptr1;

            /*if (ptr2 != ptr1)
            {
                basics::print(__func__, "BAD: Failed!", __LINE__);
                return;
            }*/
        }

        //
        {
            struct A
            {
                virtual ~A() {}
                virtual void f() { basics::print("A::f()"); }
            };

            struct B : public A
            {                
                virtual void f() override { basics::print("B::f()"); }
            };

            struct D : public B
            {
                virtual void f() override { basics::print("D::f()"); }
                virtual void d() { basics::print("D::d()"); }
            };

            struct C
            {
                virtual void f() { basics::print("C::f()"); }
            };

            /*A* a10 = new A;
            B* b10 = new B;
            a10 = b10;*/

            shared_pointer<B, true> ptr1(new B);
            shared_pointer<A, true> ptr2;
            ptr2 = ptr1;
            ptr2->f();

            if (ptr2 != ptr1)
            {
                basics::print(__func__, "BAD: Failed!", __LINE__);
                return;
            }

            shared_pointer<B, true> ptr3(new B);
            shared_pointer<A, true> ptr4(new A);

            shared_pointer<B, true> prevB(ptr3);
            shared_pointer<A, true> prevA(prevB);

            //prevA.swap(ptr4);
            swap(prevA, ptr4);

           // ptr3.swap(ptr4);
            /*ptr4.swap(ptr3);
            if (ptr3 != prevB || ptr4 != prevA)
            {
                basics::print(__func__, "BAD: Failed!", __LINE__);
               // return;
            }

            swap(ptr3, ptr4);
            swap(ptr4, ptr3);
            if (ptr3 == ptr4)
            {
                basics::print(__func__, "BAD: Failed!", __LINE__);
               // return;
            }*/


            shared_pointer<A, true> ptr5(new B);
            shared_pointer<D, true> ptr6(new D);
            //ptr6 = ptr5;
            //swap(ptr6, ptr5);
            D* d1;
            A* a1 = new A();
            //d1 = static_cast<D*>(a1);
            //ptr6->d();

            std::shared_ptr<A> ptr7(new A);
            std::shared_ptr<D> ptr8(new D);
           // ptr8 = ptr7;
        }

        {
            struct A
            {
                virtual ~A() {}
                virtual void f() { basics::print("A::f()"); }
            };

            struct B : public A
            {
                virtual void f() override { basics::print("B::f()"); }
            };

           // shared_pointer<A&> p;
        }

        basics::print(__func__, "GOOD: Test passed!");
    }

    struct Order
    {
        int qty;
        double px;
        uint64_t id;
        Order(int qty_, double px_, uint64_t id_) :qty(qty_), px(px_), id(id_)
        {
          //  _created_N_++; 
        }

        /*Order()
        {
            _created_N_++;
        }

        Order(Order&& o)
        {
            _moved_N_++;
        }

        ~Order()
        {
            _destroyed_N_++;
        }*/

        static uint32_t _destroyed_N_;
        static uint32_t _created_N_;
        static uint32_t _moved_N_;
        static uint32_t _copied_N_;
    };

    uint32_t Order::_destroyed_N_ = 0;
    uint32_t Order::_created_N_ = 0;
    uint32_t Order::_moved_N_ = 0;
    uint32_t Order::_copied_N_ = 0;

    void testSharedPtrinMultiThreadingEnv()
    {
        const size_t N = 200000;
        const size_t threadsN = 10;

        using Msg = std::tuple<Order, std::atomic<bool>, std::atomic<bool>>;
        //using Ptr = shared_pointer<Msg, false, std::memory_order_relaxed>;
        using Ptr = shared_pointer<Msg, false, std::memory_order_acq_rel>;

        std::chrono::system_clock::time_point begin = std::chrono::system_clock::now();

        std::vector<FIFOQueue<Ptr>> queues(threadsN);
      
        auto publish = [N](std::vector<FIFOQueue<Ptr>>& queues) {
            for (uint64_t i = 0; i < N; i++)
            {

                Msg* ptr = new Msg( Order(1,2.1,i), false, false);
                Ptr p(ptr);
                for_each(queues.begin(), queues.end(), [&p](auto& it) {
                    it.add(p);
                });
                p.reset();
                std::get<2>(*ptr).store(true, std::memory_order_release); // publisher release the resource!
            }
            basics::print("finished publishing messages!");
        };

        std::thread publishThread(publish, std::ref(queues));
        publishThread.join();

        std::atomic<uint64_t> _counter_(0);
        auto consume = [N, threadsN, &_counter_](int id, FIFOQueue<Ptr>& q) {
            while (_counter_.load() < N)
            {
                Ptr d;
                Ptr& m = d;
                if (!q.pop(m, false))
                    continue;

                while (!std::get<2>(*m).load(std::memory_order_acquire))
                {
                    //wait until publisher relases the resource!
                }

                bool lastOne = false;
                if (m.use_count() == threadsN)
                {
                    std::get<1>(*m).store(true, std::memory_order_release);
                    lastOne = true;
                }

                while (!std::get<1>(*m).load(std::memory_order_acquire) ||
                    m.use_count() != id)
                {
                }

                if (m.use_count() == 1) //about to be destoryed!
                    _counter_.fetch_add(1, std::memory_order_acq_rel);
                m.reset();
            }
        };

        std::list<std::thread> consumers;
        for (int i = 1; i <= threadsN; i++)
        {
            consumers.emplace_back(std::thread(
                consume, i, std::ref(queues.at(i - 1))));
        }

        for (std::thread& t : consumers)
            t.join();

        std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
        std::cout << std::endl << "Took ->" << std::chrono::duration_cast<std::chrono::milliseconds > (end - begin).count()
            << " ms to execute" << std::endl;

        for_each(queues.begin(), queues.end(), [](auto& v) {
            v.shrink(); 
        });

        basics::print(__func__, "GOOD: Test passed -> destroyed objects ", Order::_destroyed_N_,
                     "Samples", N,  "created objects ", Order::_created_N_,
                     "moved objects ", Order::_moved_N_);
    };
    
    template <typename PtrType>
    double benchmarkAllPtr()
    {
        const size_t N = 1000000;

        double r = 0;
        std::chrono::system_clock::time_point begin = std::chrono::system_clock::now();
                
        for (int i = 0; i < N; i++)
        {
            PtrType p1(new Order{ 1, 12.2, 12 });
            PtrType p2(p1);
            int qty = p2->qty;
            double px = p2->px;
            r = qty * px;
            long qt = p2.use_count();
            r = r * qt;
            PtrType p3 = p1;
            PtrType p4(std::move(p3));
            p4 = p2;
        }

       std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
        std::cout << basics::typeName<PtrType>() << __func__ << " -> " << std::endl << "Took ->" << std::chrono::duration_cast<std::chrono::milliseconds > (end - begin).count()
            << " ms to execute" << std::endl;

        return r;
    }

    template <typename PtrType>
    double benchmarkOps()
    {
        const size_t N = 1000000;

        double r = 0;
        PtrType p1(new Order{ 1, 12.2, 12 });
        std::chrono::system_clock::time_point begin = std::chrono::system_clock::now();

        for (int i = 0; i < N; i++)
        {   
            PtrType p2(p1);
            //int qty = p2->qty;
           // double px = p2->px;            
            long qt = p2.use_count();            
            PtrType p3 = p1;
            PtrType p4(std::move(p3));
            p4 = p2;
        }

        std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
        std::cout << basics::typeName<PtrType>() << __func__ << " -> " << std::endl << "Took ->" << std::chrono::duration_cast<std::chrono::milliseconds > (end - begin).count()
            << " ms to execute" << std::endl;

        return r;
    }

    void benchmark()
    {
        benchmarkAllPtr<std::shared_ptr<Order>>();
        benchmarkAllPtr<shared_pointer<Order>>();

        benchmarkOps<std::shared_ptr<Order>>();
        benchmarkOps<shared_pointer<Order>>();
    }
}