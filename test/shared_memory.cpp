










#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>

int main() {
    // создаем ключ
    key_t key = ftok("shmfile",65); 

    printf("key = %d\n", key);
    perror("ftok");
    // создаем сегмент памяти
    int shmid = shmget(key, 1024, 0666|IPC_CREAT); 
    // присоединяем сегмент
    char *str = (char*) shmat(shmid, (void*)0, 0); 

    sprintf(str, "Hello from C!");
    sleep(10);
    // ожидаем
    printf("Data written in memory: %s\n", str);
    shmdt(str); // Отсоединяем сегмент

    return 0;
}








