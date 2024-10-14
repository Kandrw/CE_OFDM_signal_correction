#include "managment_ipc.hpp"

#include <sys/socket.h>

#include <string>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <unistd.h> 
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>
#include <arpa/inet.h>
// #include <sys/sem.h>

#include <output.hpp>

#define FILE_SEM "semaphore"
#define FILE_SHARED_MEMORY "."
#define SIZE_MEMORY 20000

// #define ID 1234
// #define ID_SERVER 5678
// #define LOCK 1
#define PORT 4500
#define IP_SERVER "127.0.12.34"

struct msg_control_sm {
    u_char src;
    u_char dist;
    u_char recv;
};



static u_char *segment_shared_memory = nullptr;

msg_control_sm *control_ipc_sm = nullptr;

msg_header *msg = nullptr;

// sem_t *semaphore = nullptr;
u_char *data_segment = nullptr;

int server_fd, new_socket;
struct sockaddr_in address;

struct sockaddr_in client;
int fd_ipc;


int init_ipc() {
    print_log(LOG, "connect, file shared memory: %s\n", FILE_SHARED_MEMORY);
    key_t key = ftok(FILE_SHARED_MEMORY,65); 
    if(key < 0){
        perror("ftok");
        print_log(CONSOLE, "Error connect\n");
        return -1;
    }
    int shmid = shmget(key, SIZE_MEMORY, 0666|IPC_CREAT); 
    segment_shared_memory = (u_char*) shmat(shmid, (void*)0, 0); 

    // control_ipc_sm = (msg_control_sm*)segment_shared_memory;

    // msg = (msg_header*)(segment_shared_memory + sizeof(msg_control_sm));
    // data_segment = (segment_shared_memory + sizeof(msg_control_sm) + sizeof(msg_header));
    data_segment = segment_shared_memory;
    


    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return -1;
    }
    int setsock = 1;
    if (setsockopt(
        server_fd, SOL_SOCKET, SO_REUSEADDR, &setsock, sizeof(setsock))) 
        {
        perror("setsockopt");
        return -1;
    }

    inet_pton(AF_INET, IP_SERVER, &(address.sin_addr));
    address.sin_family = AF_INET;
    // address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if(bind(server_fd, (struct sockaddr*)&address, sizeof(address))){
        printf("errno = %d\n", errno);
        perror("bind");
        return -1;
    }
    listen(server_fd,1);
    int size = sizeof(client);
    fd_ipc = accept(server_fd, (struct sockaddr*)&client, (socklen_t*)&size);
    return 0;
}

void deinit_ipc() {
    print_log(LOG, "disconnect, file shared memory: %s\n", FILE_SHARED_MEMORY);
    if(segment_shared_memory)
        shmdt(segment_shared_memory); 
    
    close(fd_ipc);
    close(server_fd);
}


int send_ipc(u_char command, u_char type, int size, const u_char *data) {
    int status = 0;
    msg_header msg_con = msg_header(command, type, size);
    
    // for(int i = 0; i < sizeof(msg_con); ++i) {
    //     print_log(CONSOLE, "%d) %x %d\n",i,
    //      *((char*)(&msg_con) + i),  *((char*)(&msg_con) + i));
    // }
    // print_log(CONSOLE, "\nsizeof(msg_con) = %d\n", sizeof(msg_con));
    // u_char *buf = &msg_con;
    if(send(fd_ipc, (void*)&msg_con, sizeof(msg_header), 0) == -1){
        perror("send");
        return -1;
    }
    return status;
}
int recv_ipc(msg_header *msg_n, int size, u_char *data) {
    // print_log(CONSOLE, "%s:%d\n", __func__, __LINE__);
    if(recv(fd_ipc, (void*)msg_n, sizeof(msg_header), 0) == -1){
        perror("recv");
        return -1;
    }
    // u_char buffer[20];
    // if(recv(fd_ipc, (void*)buffer, sizeof(buffer), 0) == -1){
    //     perror("recv");
    //     return -1;
    // }
    // for(int i = 0; i < 20; ++i) {
    //     print_log(CONSOLE, "%x - %d\n", buffer[i], buffer[i]);
    // }
    return 0;
}


// int full_data_arrays(std::string str, u_char count_arrays, ...) {
//     va_list factor;
//     int size_curr = 4 + str.size();
//     print_log(CONSOLE, "%s:%d\n", __func__, __LINE__);
//     va_start(factor, count_arrays);
//     for(int i = 0; i < count_arrays; i++) {
//         print_log(CONSOLE, "%s:%d\n", __func__, __LINE__);
//         u_char size_str = va_arg(factor, u_char);
//         print_log(CONSOLE, "%s:%d\n", __func__, __LINE__);
//         const char *str = va_arg(factor, char*);
//         unsigned int size_array = va_arg(factor, unsigned int);
//         u_char type = va_arg(factor, u_char);
//         u_char *array = va_arg(factor, u_char*);
//         print_log(CONSOLE, "%s:%d\n", __func__, __LINE__);
//         size_curr += 1;
//     }
//     print_log(CONSOLE, "%s:%d\n", __func__, __LINE__);
//     va_end(factor);
// }

#define CPY_SM_AND_STEP(ptr, mem, size) \
memcpy(ptr, mem, size);\
ptr += size;

#define CPY_SM_1_BYTE_AND_STEP(ptr, val) \
*ptr = val;\
ptr++;

int full_data_arrays(std::string str,
    std::vector<data_array*> &arrays) 
{
    va_list factor;
    int size_curr = 4 + str.size();
    print_log(CONSOLE, "%s:%d\n", __func__, __LINE__);
    u_char *ptr_cpy_sm = data_segment;
    int size = str.size();
    CPY_SM_AND_STEP(ptr_cpy_sm, (u_char*)&size, 4);
    if(str.size() > 0)
        CPY_SM_AND_STEP(ptr_cpy_sm, (u_char*)str.c_str(), str.size());
    // u_char count_arr = arrays.size();
    // CPY_SM_AND_STEP(ptr_cpy_sm, (u_char*)&count_arr, 1);
    CPY_SM_1_BYTE_AND_STEP(ptr_cpy_sm, arrays.size());
    // u_char size_size_str = 1;
    for(int i = 0; i < arrays.size(); i++) {
        size_curr += 1 + arrays[i]->size_str + 4 + 1 + arrays[i]->size_array;
        if(size_curr >= SIZE_MEMORY) {
            print_log(ERROR_OUT, "The record size is larger than possible:"
                " size shared memory = %d\n", SIZE_MEMORY);
            return size_curr;
        }
        print_log(CONSOLE, "%s:%d\n", __func__, __LINE__);
        // ptr_cpy_sm[0]
        CPY_SM_1_BYTE_AND_STEP(ptr_cpy_sm, arrays[i]->size_str);
        CPY_SM_AND_STEP(ptr_cpy_sm, (u_char*)arrays[i]->str, arrays[i]->size_str);
        size = arrays[i]->size_array;
        CPY_SM_AND_STEP(ptr_cpy_sm, (u_char*)&size, 4);
        CPY_SM_1_BYTE_AND_STEP(ptr_cpy_sm, arrays[i]->type);
        CPY_SM_AND_STEP(ptr_cpy_sm, (u_char*)arrays[i]->array, arrays[i]->size_array);
        // ptr_cpy_sm = data_segment + size_curr;
        
    }
    print_log(CONSOLE, "%s:%d size_curr = %d\n", __func__, __LINE__, size_curr);
    return size_curr;
}



