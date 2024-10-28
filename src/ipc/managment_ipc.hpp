#pragma once
#include <string>

#include "../header.hpp"
#ifndef ACTIVATE_IPC
#define ACTIVATE_IPC
#endif
#define IPC_SIZE_BUFFER 64

enum class COMMAND_IPC_MSG {
    END = 10,
    VIEW_DATA_PLOT_XY,
    
};

enum class TYPE_ARRAY {
    TYPE_INT = 0b1,
    TYPE_FLOAT,
    TYPE_COMPLEX_FLOAT
};

struct data_array {
    u_char size_str;
    const u_char *str;
    unsigned int size_array;
    u_char type;
     u_char *array;
    data_array(u_char size_str,
    const u_char *str,
    unsigned int size_array,
    u_char type,
    u_char *array) : size_str(size_str), str(str), size_array(size_array), type(type), array(array){}
    data_array() {}
};



struct msg_header {
    u_char command = 0;
    u_char type = 0;
    int size_data_shm = 0;
    // int size_data;
    // u_char data[IPC_SIZE_BUFFER];
    msg_header() {}
    msg_header(u_char command, u_char type, int size_data_shm) : 
        command(command), type(type), size_data_shm(size_data_shm) {}
}__attribute__ ((__packed__));


int init_ipc();
void deinit_ipc();
int send_ipc(u_char command, u_char type, int size, const u_char *data);
int recv_ipc(msg_header *msg_n, int size, u_char *data);
// int full_data_arrays(std::string str, u_char count_arrays, ...);
int full_data_arrays(
    std::string str,
    std::vector<data_array*> &arrays);


int test_ipc(int argc, char *argv[]);

// 1) 1 - lock 0 - unlock
// 2) 0        1

/*

protocol in tcp

command - VIEW, END
type - 
size - shared memory


shared memory
type - char, int, u_int, float, double, complex float, complex double

size str 4 b
str 0 - 2^32 b
count array 1 b
    size str 1 b
    str 0-255 b
    size - 4 b
    type - 1 b
    array1 0 - 2^32 b


*/
