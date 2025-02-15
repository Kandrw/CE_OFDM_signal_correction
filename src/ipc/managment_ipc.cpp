#include "managment_ipc.hpp"

#include <sys/socket.h>

#include <iostream>
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
#include <output.hpp>

/*Shared memory: System V*/

#define FILE_SEM "semaphore"
#define FILE_SHARED_MEMORY "."
#define SIZE_MEMORY (int)1e7

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
u_char *data_segment = nullptr;
int server_fd, new_socket;
struct sockaddr_in address;

struct sockaddr_in client;
int fd_ipc;


int init_ipc() {
    print_log(LOG, "connect, file shared memory: %s\n", FILE_SHARED_MEMORY);
    key_t key = ftok(FILE_SHARED_MEMORY,65); 
    if(key < 0) {
        perror("ftok");
        print_log(CONSOLE, "Error connect\n");
        return -1;
    }
    int shmid = shmget(key, SIZE_MEMORY, 0666|IPC_CREAT); 
    if(shmid < 0) {
        print_log(ERROR_OUT, "Error create shared memory\n");
        perror("shmget");
        return -1;
    }
    segment_shared_memory = (u_char*) shmat(shmid, (void*)0, 0); 
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
    if(send(fd_ipc, (void*)&msg_con, sizeof(msg_header), 0) == -1){
        perror("send");
        return -1;
    }
    return status;
}
int recv_ipc(msg_header *msg_n, int size, u_char *data) {
    if(recv(fd_ipc, (void*)msg_n, sizeof(msg_header), 0) == -1){
        perror("recv");
        return -1;
    }
    return 0;
}

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
    // print_log(CONSOLE, "%s:%d\n", __func__, __LINE__);
    u_char *ptr_cpy_sm = data_segment;
    int size = str.size();
    CPY_SM_AND_STEP(ptr_cpy_sm, (u_char*)&size, 4);
    if(str.size() > 0)
        CPY_SM_AND_STEP(ptr_cpy_sm, (u_char*)str.c_str(), str.size());
    CPY_SM_1_BYTE_AND_STEP(ptr_cpy_sm, arrays.size());
    for(int i = 0; i < arrays.size(); i++) {
        size_curr += 1 + arrays[i]->size_str + 4 + 1 + arrays[i]->size_array;
        if(size_curr >= SIZE_MEMORY) {
            print_log(ERROR_OUT, "The record size is larger than possible:"
                " size shared memory = %d, write - %d\n", SIZE_MEMORY, size_curr);
            return size_curr;
        }
        // print_log(CONSOLE, "%s:%d\n", __func__, __LINE__);
        CPY_SM_1_BYTE_AND_STEP(ptr_cpy_sm, arrays[i]->size_str);
        CPY_SM_AND_STEP(ptr_cpy_sm, (u_char*)arrays[i]->str, arrays[i]->size_str);
        size = arrays[i]->size_array;
        CPY_SM_AND_STEP(ptr_cpy_sm, (u_char*)&size, 4);
        CPY_SM_1_BYTE_AND_STEP(ptr_cpy_sm, arrays[i]->type);
        CPY_SM_AND_STEP(ptr_cpy_sm, (u_char*)arrays[i]->array, arrays[i]->size_array);
    }
    // print_log(CONSOLE, "%s:%d size_curr = %d\n", __func__, __LINE__, size_curr);
    return size_curr;
}

int test_ipc(int argc, char *argv[]) {

#ifdef ACTIVATE_IPC
    if(init_ipc()) {
        print_log(CONSOLE, "Exit\n");
        return -1;
    }
#endif
    float FF[] = {12, 2, 21, 3};
    int FF2[] = {10, 20, 30, 40};
    
    std::vector<data_array*> arr;
    char df[] = "asd";
    data_array d1 = data_array((int)3, (const u_char*)df, (unsigned int)sizeof(FF), 
            static_cast<u_char>(TYPE_ARRAY::TYPE_FLOAT), (u_char*)FF);
    data_array d2 = data_array((int)3, (const u_char*)df, (unsigned int)sizeof(FF), 
            static_cast<u_char>(TYPE_ARRAY::TYPE_INT), (u_char*)FF2);
     
    arr.push_back(&d2);                        
    arr.push_back(&d1);
    // data_array d1(3, (u_char*)df, sizeof(FF), TYPE_ARRAY::TYPE_FLOAT, (u_char*)FF);
    // data_array d1;// = {3, (u_char*)df, sizeof(FF), TYPE_ARRAY::TYPE_FLOAT, (u_char*)FF};
    // d1.array = (u_char*)FF;
    // d1.size_array = sizeof(FF);
    // d1.size_str = 3;
    // d1.str = (const u_char*)df;
    // d1.type = TYPE_ARRAY::TYPE_FLOAT;
    time_counting_start();
    full_data_arrays("asd", arr);
    time_counting_end(CONSOLE, __func__);
    print_log(CONSOLE, "%s:%d\n", __func__, __LINE__);
    // return 0;
    while(1) {
        int command;
        print_log(CONSOLE, "%s:%d: input:", __func__, __LINE__);
        std::cin >> command;

        if(command) {
            print_log(CONSOLE, "send\n");
            std::string asd = "asdfasfdsdf";
            send_ipc(command, 4, 12, (u_char*)asd.c_str());
            if(command == 10) {
                break;
            }
#if 0
            msg_header msg;
            recv_ipc(&msg, 0, NULL);
            for(int i = 0; i < sizeof(msg); ++i) {
        print_log(CONSOLE, "%d) %x %d\n",i,
         *((char*)(&msg) + i),  *((char*)(&msg) + i));
    }
    print_log(CONSOLE, "\nsizeof(msg_con) = %d\n", sizeof(msg));
            print_log(CONSOLE, "c = %d, t = %d, s = %d\n", msg.command, msg.type, msg.size_data_shm);
#endif
        }
    }
    deinit_ipc();
    return 0;
}
