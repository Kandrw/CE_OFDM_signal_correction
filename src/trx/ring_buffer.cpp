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

#define DEBUG_RINGS

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

void RingBuffer::lock() {
    while (lock_flag.test_and_set(std::memory_order_acquire)) {}
}

void RingBuffer::unlock() {
    lock_flag.clear(std::memory_order_release);
}

void* RingBuffer::get_pointer_push() {
    return buffer[index_push];
}

void* RingBuffer::get_pointer_pop() {
    return buffer[index_pop];
}

void RingBuffer::up_index() {
    index_push++;
    count++;
    if(index_push == size) {
        index_push = 0;
    }
}

void RingBuffer::down_index() {
    index_pop++;
    count--;
    if(index_pop == size) {
        index_pop = 0;
    }
}

int RingBuffer::get_size_element() {
    return size_el;
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

int RingBuffer::is_full() {
    int status = 0;
    if(count == size - 1) {
        status = 1;
    }
    return status;
}

int RingBuffer::is_empty() {
    int status = 0;
    if(count == 0) {
        status = 1;
    }
    return status;
}

RingMsg::RingMsg(int size, int size_block, int size_type) : RingBuffer(size, size_block, size_type) {}

int RingMsg::Pop(msg_buffer *msg) {
    void *msg_ptr;
    lock();
    if(is_empty()) {
        unlock();
        return 0;
    }
    msg_ptr = get_pointer_pop();
    memcpy((void*)msg, msg_ptr, sizeof(msg_buffer));
    memset(msg_ptr, 0, sizeof(msg_buffer));
    print_log(CONSOLE, "[RingMsg:%s:%d]\n", __func__, __LINE__);
    down_index();
    unlock();
    return 1;
}

int RingMsg::Push(const msg_buffer *msg) {
    void *msg_ptr;
    lock();
    if(is_full()) {
        unlock();
        return 0;
    }
    msg_ptr = get_pointer_push();
    memcpy(msg_ptr, (void*)msg, sizeof(msg_buffer));
    ((msg_buffer*)msg_ptr)->data = new u_char[msg->size];
    memcpy((void*)((msg_buffer*)msg_ptr)->data, msg->data, msg->size);
    print_log(CONSOLE, "[RingMsg:%s:%d]\n", __func__, __LINE__);
    up_index();
    unlock();
    return 1;
}

RingSamples::RingSamples(int size, int size_block, int size_type) : RingBuffer(size, size_block, size_type) {}

int RingSamples::Pop(void *data) {
    void *data_ptr;
    lock();
    if(is_empty()) {
        unlock();
        return 0;
    }
    data_ptr = get_pointer_pop();
    memcpy(data, data_ptr, get_size_element());
    memset(data_ptr, 0, get_size_element());
    print_log(CONSOLE, "[RingSamples:%s:%d][1]\n", __func__, __LINE__);
    down_index();
    unlock();
    return 1;
}

int RingSamples::Pop(int (*get_data)(const void *, size_t)) {
    void *data_ptr;
    lock();
    if(is_empty()) {
        unlock();
        return 0;
    }
    data_ptr = get_pointer_pop();
    phy_buffer *msg_phy = (phy_buffer*)data_ptr;
    get_data(data_ptr + sizeof(phy_buffer), msg_phy->size);
    // memset(data_ptr, 0, get_size_element());
    print_log(CONSOLE, "[RingSamples:%s:%d][2] size = %d\n", __func__, __LINE__, msg_phy->size);
    msg_phy->size = 0;
    down_index();
    unlock();
    return 1;
}

int RingSamples::Push(const void *data) {
    void *data_ptr;
    lock();
    if(is_full()) {
        unlock();
        return 0;
    }
    phy_buffer *msg_phy = (phy_buffer*)data;
    data_ptr = get_pointer_push();
    memcpy(data_ptr, data,
        msg_phy->size + sizeof(phy_buffer));
    print_log(CONSOLE, "[RingSamples:%s:%d][1]\n", __func__, __LINE__);
    up_index();
    unlock();
    return 1;
}

int RingSamples::Push(int (*get_data)(void *, size_t)) {
    void *data_ptr;
    lock();
    if(is_full()) {
        unlock();
        return 0;
    }
    data_ptr = get_pointer_push();
    phy_buffer *msg_phy = (phy_buffer*)data_ptr;
    get_data(data_ptr + sizeof(phy_buffer), get_size_element());
    msg_phy->size = get_size_element();
    print_log(CONSOLE, "[RingSamples:%s:%d][2] size = %d\n", __func__, __LINE__, msg_phy->size);
    up_index();
    unlock();
    return 1;
}
