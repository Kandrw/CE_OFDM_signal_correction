#include "ring_buffer.hpp"


#include <errno.h>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <thread>
#include <atomic>

#include <output.hpp>
#include <complex_container.hpp>
#include <signal_processing.hpp>



RingBuffer::RingBuffer(int size, int size_block, int size_type) :
    size(size), size_block(size_block),
    size_type(size_type), size_el(size_block * size_type)
{
    buffer = (void**)calloc(size, sizeof(void*));
    for(int i = 0; i < size; ++i) {
        buffer[i] = calloc(size_block, size_type);
    }
    count = 0;
    index_pop = 0;
    index_push = 0;
}

int RingBuffer::Pop(void *el) {
    while (lock_flag.test_and_set(std::memory_order_acquire)) {}
    if(!count) {
        return 0;
    }
    memcpy(el, buffer[index_pop], size_el);
    index_pop++;
    count--;
    if(index_pop == size) {
        index_pop = 0;
    }
    lock_flag.clear(std::memory_order_release);
    return 1;
}

int RingBuffer::Push(const void *el) {
    while (lock_flag.test_and_set(std::memory_order_acquire)) {}
    if(count == size - 1) {
        return 0;
    }
    memcpy(buffer[index_push], el, size_el);
    index_push++;
    count++;
    if(index_push == size) {
        index_push = 0;
    }
    lock_flag.clear(std::memory_order_release);
    return 1;
}








