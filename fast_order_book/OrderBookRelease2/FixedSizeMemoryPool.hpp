#ifndef _FIXED_SIZE_MEMORY_POOL_HPP_
#define _FIXED_SIZE_MEMORY_POOL_HPP_

#include "Common.hpp"
#include <stdexcept>
#include <stdlib.h>

class FixedSizeMemoryPool
{
public:
    static FixedSizeMemoryPool& instance(int objSize, int preallocSize = FIXED_ALLOCATOR_SIZE)
    {
        static FixedSizeMemoryPool pool(objSize, preallocSize);
        return pool;
    }

    void* allocate()
    {
        if (unlikely(m_freeList == 0))
        {
            throw std::runtime_error("Memory is over!Bye Bye!");
        }

        Memory* freeItem = m_freeList;
        m_freeList = m_freeList->next;
        return static_cast<void*>((freeItem + sizeof(Memory)));
    }

    void deallocate(void* address)
    {
        Memory* busyItem = static_cast<Memory*>(address - sizeof(Memory));    
        busyItem->next = m_freeList;
        m_freeList = busyItem;
    }

private:
    FixedSizeMemoryPool(int objSize, int preallocSize) 
       : m_freeList(0) 
    {
        int itemSize = sizeof(Memory) + objSize;
        m_contigousMemory.resize(itemSize*preallocSize, 0);

        Memory* prev = 0;
        for (int i = 0; i < preallocSize; i++)
        {
            m_freeList = reinterpret_cast<Memory*>(&m_contigousMemory[itemSize * i]);
            m_freeList->next = prev;
            prev = m_freeList;
        }
    }

private:
    struct Memory
    {
        Memory* next;
    };

    Memory* m_freeList;
    std::vector<char*> m_contigousMemory;
};

#endif // _FIXED_SIZE_MEMORY_POOL_HPP_
