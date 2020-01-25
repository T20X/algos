#ifndef _LATENCY_MONITOR_HPP_
#define _LATENCY_MONITOR_HPP_

#include <vector>
#include <ostream>
#include <stdint.h>
#include <ext/hash_map>

#include "Config.hpp"

// It will work reliably if your CPU supports invariant TSC, power management is off 
// and you don't have multiple CPU packages. Not thread-safe atm, but will do 
// for the current usage patterns
  
class LatencyMonitor
{
public:
    static LatencyMonitor& instance() 
    {
        static LatencyMonitor singleton;
        return singleton;
    }

    static uint64_t tick() 
    {
        unsigned hi, lo;
        __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
        return ( (uint64_t)lo)|( ((uint64_t)hi)<<32 );
    }

    void begin(const char* key, uint64_t& from)
    {
        from = tick();
    }

    void end(const char* key, const uint64_t& from)
    {
        uint64_t latency = tick() - from;
        SampleStat& stat = m_allStat[key];
        {
            stat.avg  = (stat.avg * stat.N + latency) /
                        (stat.N + 1);
            stat.N++;
        }
    }

    void dumpStat(std::ostream& o)
    {
        for (SampleMap::const_iterator it = m_allStat.begin();
             it != m_allStat.end();
             it++)
        {
            o << "\n" 
              << it->first << " - " 
              << " Avg[" << it->second.avg 
              << " CPU ticks] N[" << it->second.N 
              << "]";
        }
    }

private:
    LatencyMonitor()
    {
        m_allStat.resize(LATENCY_MONITOR_ALLOC_SIZE);
    }
    LatencyMonitor(const LatencyMonitor&);
    LatencyMonitor& operator=(const LatencyMonitor&);

private:
    struct SampleStat
    {
        uint64_t avg;
        uint64_t N;
    
        SampleStat():avg(0),N(0){}
    };   

    typedef __gnu_cxx::hash_map<const char*, SampleStat > SampleMap;
    SampleMap m_allStat;
    
};

#ifdef TRACE_LATENCY 
#define BEGIN_LATENCY_TRACE(key) \
    uint64_t timestamp; \
    LatencyMonitor::instance().begin(key, timestamp); 
    #endif 

#ifdef TRACE_LATENCY 
#define END_LATENCY_TRACE(key) \
    LatencyMonitor::instance().end(key, timestamp); 
    #endif 

#ifndef TRACE_LATENCY 
   #define BEGIN_LATENCY_TRACE(key) {}
#endif

#ifndef TRACE_LATENCY 
    #define END_LATENCY_TRACE(key) {}
#endif
 

#endif // _LATENCY_MONITOR_HPP_

