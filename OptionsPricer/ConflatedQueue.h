#pragma once

#include <atomic>
#include <unordered_set>
#include "SPSCQueue.h"

template <typename T>
class ConflatedQueue
{
public:
    ConflatedQueue(size_t N):Q(N)
    {
        presentInQ.resize(N, 0);
    }

    void add(T value)
    {

    }

    T extract()
    {

    }

private:
    SPSCQueue<T> Q;
    vector<uint64_t> presentInQ;
};
