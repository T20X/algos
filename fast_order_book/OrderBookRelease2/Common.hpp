#ifndef _COMMON_HPP_
#define _COMMON_HPP_

#include <math.h>
#include <sched.h>
#include <stdint.h>
#include <iostream>
#include "Config.hpp"

/*************************************************************
 */
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

/*************************************************************
 */
inline void full_compiler_fence()
{
    __asm__ __volatile__ ("" ::: "memory");
}

/*************************************************************
 */
inline void cpuPause(void)
{
    __asm__ __volatile("pause" ::: "memory");
}

/*************************************************************
 */
inline void setBitTo1(size_t pos, volatile uint64_t *addr)
{
    // even though we work with 64-bit addr, GCC will add rex.w preffix automatically
    __asm__ __volatile("bts %1,%0" : "=m" (*(volatile long *) (addr)) : "Ir" (pos));
}

/*************************************************************
 */
inline void setBitTo0(size_t pos, volatile uint64_t *addr)
{
    // even though we work with 64-bit addr, GCC will add rex.w preffix automatically
    __asm__ __volatile("btr %1,%0" : "=m" (*(volatile long *) (addr)) : "Ir" (pos));
}

/************************************************************************************
 * Check bits for 0 before calling this function, otherwise behaviour will be undefined
 */
inline uint64_t findLeast1Bit(uint64_t bits)
{
    // even though we work with 64-bit addr, GCC will add rex.w preffix automatically
    __asm__ __volatile("bsf %1,%0" : "=r" (bits) : "Jrm" (bits));
    return bits;
}

/************************************************************************************
 * Check bits for 0 before calling this function, otherwise behaviour will be undefined
 */
inline uint64_t findMost1Bit(uint64_t bits)
{
    // even though we work with 64-bit addr, GCC will add rex.w preffix automatically
    __asm__ __volatile("bsr %1,%0" : "=r" (bits) : "Jrm" (bits));
    return bits;
}

/*************************************************************
 */
inline bool isPriceEqual(double left, double right)
{ 
    return fabs(left - right) < PRICE_EPSILON; 
}

/*************************************************************
 */
inline bool isLeftGreater(double left, double right)
{ 
    return !isPriceEqual(left, right) && left > right;
}

/*************************************************************
 */
inline bool isLeftLess(double left, double right)
{ 
    return !isPriceEqual(left, right) && left < right;
}

/*************************************************************
 */
inline void getAffinity(cpu_set_t* affinity, int n)
{
    for (int i = 0; i < n; i++)
    {
        CPU_ZERO(&affinity[i]);
    }

    cpu_set_t active_cpus;
    CPU_ZERO(&active_cpus);
    sched_getaffinity(0, sizeof(active_cpus), &active_cpus);

    for (int i = 0; i != CPU_SETSIZE && n > 0; i++)
    {
        if (CPU_ISSET(i, &active_cpus))
        {
            CPU_SET(i, &affinity[n-1]);
            n--;
        }
    }
        
    if (n != 0)
    {
        std::cerr << "\nAffinity request was not filled for "
                  << n << " threads";
    }

    for (int i = 0; i < n; i++)
    {
        //assing a default affinity to remaining threads
        affinity[i] = active_cpus;
    }
}

#endif //_COMMON_HPP_

