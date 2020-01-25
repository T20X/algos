#ifndef _ALLOCATOR_HPP_
#define _ALLOCATOR_HPP_

#include <limits>
#include <iostream>
#include <exception>

#include "FixedSizeMemoryPool.hpp"
#include "LatencyMonitor.hpp"

template <class T>
class STLCustomAllocator 
{
public:
   typedef T        value_type;
   typedef T*       pointer;
   typedef const T* const_pointer;
   typedef T&       reference;
   typedef const T& const_reference;
   typedef std::size_t    size_type;
   typedef std::ptrdiff_t difference_type;

   template <class U>
   struct rebind 
   {
       typedef STLCustomAllocator<U> other;
   };

   pointer address (reference value) const 
   {
       return &value;
   }
   const_pointer address (const_reference value) const
   {
       return &value;
   }

   STLCustomAllocator() {}
   STLCustomAllocator(const STLCustomAllocator&) throw() {} 

   template <class U>
   STLCustomAllocator (const STLCustomAllocator<U>&) throw() {}

   ~STLCustomAllocator() throw() {}

   size_type max_size () const throw() 
   {
       return std::numeric_limits<std::size_t>::max() / sizeof(T);
   }

   pointer allocate (size_type num, const void* = 0)
   {
        if (unlikely(num > 1))
        {
            throw std::runtime_error("Allocator is tuned to allocate "
                                     "only 1 item pre request");
        }
     
        pointer ret = static_cast<pointer>(FixedSizeMemoryPool::instance(sizeof(T)).allocate());
        return ret;
   }

   void construct (pointer p, const T& value) 
   {
       // initialize memory with placement new
       new (static_cast<void*>(p)) T(value);
   }

   void destroy (pointer p) 
   {
   //    p->~T();
   }

   void deallocate (pointer p, size_type num)
   {
        if (unlikely(num > 1))
        {
            throw std::runtime_error("Allocator is tuned to deallocate "
                                     "only 1 item pre request");
        }

       FixedSizeMemoryPool::instance(num).deallocate(static_cast<void*>(p));
   }

};

template <class T1, class T2>
bool operator== (const STLCustomAllocator<T1>&,
                const STLCustomAllocator<T2>&) throw() 
{
   return true;
}

template <class T1, class T2>
bool operator!= (const STLCustomAllocator<T1>&,
                 const STLCustomAllocator<T2>&) throw() 
{
   return false;
}

#endif // _ALLOCATOR_HPP_

