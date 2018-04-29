#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <sys/shm.h>
#include "../headers/coms.h"
#include "../headers/inits.h"
#include "../headers/launcherUtils.h"
#include "../headers/ticketUtils.h"

#define SHM_KEY 20000

void initMailBoxes(int nodeID) {
    if (msgget(nodeID + NODE_REQUEST_BASE, 0666 | IPC_CREAT) == -1 || msgget(nodeID + NODE_REPLY_BASE, 0666 | IPC_CREAT) == -1)
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

    ticket ticket;
    ticket.nodeID = nodeID;
    ticket.requestID = 0;

    sharedMemoryPointer->wantTo = 0;
    sharedMemoryPointer->maxPetition = 0;
    sharedMemoryPointer->pendingRequestsCount = 0;
    sharedMemoryPointer->myTicket = ticket;

    return sharedMemoryPointer;
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