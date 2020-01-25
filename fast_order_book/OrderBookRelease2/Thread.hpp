#ifndef _THREAD_HPP_
#define _THREAD_HPP_

#include <pthread.h>
#include <sched.h>
#include <string>
#include <stdexcept>

class Thread
{
protected:
    Thread(const std::string& name, cpu_set_t* affinity)
        : m_name(name)
    {
        if (pthread_attr_init(&m_thread_attr))
        {
           throw std::runtime_error("Unable to initialize thread attributes");
        }

        if (affinity != 0)
        {
           pthread_attr_setaffinity_np(
               &m_thread_attr,
                sizeof(cpu_set_t), 
                affinity);
        }
        
        std::cout << "Thread " << m_name.c_str() 
                  << " initialized" << std::endl;
    }

    void create()
    {
        if (pthread_create(&m_thread, 0, &call_process, this))
        {
            throw std::runtime_error("Unable to create a thread");
        }

        std::cout << "Thread " << m_name.c_str() 
                  << " created" << std::endl;
    }
    
    void join()
    {
         pthread_join(m_thread, 0);
         pthread_attr_destroy(&m_thread_attr);

        std::cout << "Thread " << m_name.c_str() 
                  << " stopped" << std::endl;
    }

    void cancel()
    {
        pthread_cancel(m_thread);
        std::cout << "Thread " << m_name.c_str() 
                  << " canceled" << std::endl;
    }

    virtual void* process() = 0;

private:
    Thread(const Thread& another);
    Thread& operator == (const Thread& another);

    static void* call_process(void* me)
    {
        try
        {   
            return static_cast<Thread*>(me)->process();
        }
        catch(const std::exception& e)
        {
            std::cerr << "\nException while running thread " 
                      << " [" << e.what() << "]";
        }
        catch(...)
        {
            std::cerr << "\n Unknown exception while running thread";
        }

        return 0;
    }

protected:
    std::string     m_name;
    pthread_t       m_thread;
    pthread_attr_t  m_thread_attr;
};

#endif // _THREAD_HPP_

