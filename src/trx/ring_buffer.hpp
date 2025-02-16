#pragma once

#include <cstdlib>
#include <unistd.h>
#include <thread>
#include <atomic>

#include "types_trx.hpp"

class RingBuffer {
public:

    RingBuffer(int size, int size_block, int size_type);
    int Pop(void *el);
    int Push(const void* el);
    void lock();
    void unlock();
    void* get_pointer_pop();
    void* get_pointer_push();
    int is_full();
    int is_empty();
    void up_index();
    void down_index();
    int get_size_element();
private:
    int index_push, index_pop;
    int count;
    std::atomic_flag lock_flag = ATOMIC_FLAG_INIT;
    void **buffer;
    int size, size_block, size_type;
    int size_el;
};

class RingMsg : public RingBuffer {
public:
    RingMsg(int size, int size_block, int size_type);
    int Pop(msg_buffer *msg);
    int Push(const msg_buffer *msg);
};

class RingSamples : public RingBuffer {
public:
    RingSamples(int size, int size_block, int size_type);
    int Pop(void *data);
    int Pop(int (*get_data)(const void *, size_t));
    int Push(const void *data);
    int Push(int (*get_data)(void *, size_t));
};
