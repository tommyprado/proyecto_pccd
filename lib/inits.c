#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <sys/shm.h>
#include "../headers/inits.h"

#define NODE_INITIAL_KEY 10000
#define SHM_KEY 20000
#define WRITE_OUT_QUEUE 283300

void initMailBoxes(int nodeID) {
    int key= nodeID + NODE_INITIAL_KEY;
    int msqid = msgget(key, 0666 | IPC_CREAT);
    if (msqid == -1)
    {
        printf("Error buzón\n");
        exit (-1);
    }

    msqid = msgget(WRITE_OUT_QUEUE, 0666 | IPC_CREAT);
    if (msqid == -1)
    {
        printf("Error buzón\n");
        exit (-1);
    }
}

void initSemaphore(sem_t *semaphore) {
    int err = sem_init(semaphore, 0, 1);
    if (err) {
        printf("Error inicializando semáforos\n");
        exit(1);
    }
}

sharedMemStruct *initSharedMemory(int nodeID) {
    int key = SHM_KEY + nodeID;
    int id = shmget(key, sizeof(sharedMemStruct), IPC_CREAT | 0666);
    if (id < 0) {
        printf("shmget error\n");
        exit(1);
    }

    sharedMemStruct *sharedMemoryPointer;
    sharedMemoryPointer = (sharedMemStruct *) shmat(id, NULL, 0);

    initSemaphore(&sharedMemoryPointer->semTicket);
    initSemaphore(&sharedMemoryPointer->semWantTo);
    initSemaphore(&sharedMemoryPointer->semPending);
    initSemaphore(&sharedMemoryPointer->semMaxPetition);
}

sharedMemStruct *getSharedMemory(int nodeID) {
    int key = SHM_KEY + nodeID;
    int id = shmget(key, sizeof(sharedMemStruct), IPC_CREAT | 0666);
    if (id < 0) {
        printf("shmget error\n");
        exit(1);
    }
    return (sharedMemStruct *) shmat(id, NULL, 0);
}