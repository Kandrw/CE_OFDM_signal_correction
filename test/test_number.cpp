#include <iostream>
#include <cstdlib>
#include <cstring>

#define BYTE 8
typedef unsigned char u_char;

void print_bit(u_char *buffer, int size){
    int i, i2;
    u_char iter;
    for(i = 0; i < size; ++i){
        iter = buffer[i];
        for(i2 = 0; i2 < BYTE; ++i2){
            switch(iter & 128){
            case 0:
                printf("0");
                break;
            default:
                printf("1");
                break;
            } 
            iter <<= 1;  
        }
        printf(" ");
    }
    printf("\n");
}

void test1() {
    volatile int16_t r = 0;
    volatile float rf;
    volatile int err = 0;
    for(int i = 0; i < 10000000; i++) {
        r = r + 1;

        rf = r;
        if((float)r != (float)rf) {
            err++;
        }
    }
    std::cout<<"err = "<<err<<"\n";
}

int main() {
    std::cout<<"Start\n";

    int16_t r = 0b11111111 + (0b01111111 << 8);
    float rf = 0.f;
    memcpy( ((void*)&rf) + 1, (void*)&r, 2);
    // rf = r;

    print_bit((u_char*)&r, 2);
    printf("%d\n", r);
    std::cout<<r<<"\n";


    print_bit((u_char*)&rf, 4);
    printf("\n");
    printf("%f\n", rf);
    std::cout<<rf<<"\n";
    test1();

    std::cout<<"End\n";
    return 0;
}



