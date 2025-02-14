#pragma once

#include <cstdlib>
#include <unistd.h>
#include <thread>
#include <atomic>

class RingBuffer {
    public:

    RingBuffer(int size, int size_block, int size_type);
    int Pop(void *el);
    int Push(const void* el);

    private:
    int index_push, index_pop;
    int count;
    std::atomic_flag lock_flag = ATOMIC_FLAG_INIT;
    void **buffer;
    int size, size_block, size_type;
    int size_el;
};
