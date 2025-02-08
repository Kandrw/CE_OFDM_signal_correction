



#include <cstdlib>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <cstring>
#include <ctime>
#include <iostream>


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
        printf("FULL\n");
        lock_flag.clear(std::memory_order_release);
        return 0;
    }
    printf("POP\n");
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
        printf("EMPTY\n");
        lock_flag.clear(std::memory_order_release);
        return 0;
    }
    printf("PUSH\n");
    memcpy(buffer[index_push], el, size_el);
    index_push++;
    count++;
    if(index_push == size) {
        index_push = 0;
    }
    lock_flag.clear(std::memory_order_release);
    return 1;
}
#define SIZE 30
RingBuffer *ring1;

void thr1() {
    char buffer1[SIZE] = "TTTT";
    char buffer2[SIZE] = "";
    
    while(true) {
        ring1->Push(buffer1);

        ring1->Pop(buffer2);
        printf("T1: %s\n", buffer2);
        usleep(600);
    }
}

void thr2() {
    char buffer1[SIZE] = "BBBB";
    char buffer2[SIZE] = "";
    
    while(true) {
        ring1->Push(buffer1);

        ring1->Pop(buffer2);
        printf("T2: %s\n", buffer2);
        usleep(100);
    }
}



int main() {

    ring1 = new RingBuffer(5, SIZE, 1);


    std::thread t1(thr1);
    std::thread t2(thr2);
    while(true) {

    }
    return 0;

}





