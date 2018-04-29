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

void initSemaphore(sem_t *semaphore, int limit) {
    int err = sem_init(semaphore, 0, limit);
    if (err) {
        printf("Error inicializando semáforos\n");
        exit(1);
    }
}

sharedMemory *initSharedMemory(int nodeID) {
    int key = SHM_KEY + nodeID;
    int id = shmget(key, sizeof(sharedMemory), IPC_CREAT | 0666);
    if (id < 0) {
        printf("shmget error\n");
        exit(1);
    }

    sharedMemory *sharedMemoryPointer;
    sharedMemoryPointer = (sharedMemory *) shmat(id, NULL, 0);

    initSemaphore(&sharedMemoryPointer->nodeStatusSem, 1);
    initSemaphore(&sharedMemoryPointer->competitorTicketSem, 1);
    initSemaphore(&sharedMemoryPointer->allowNextCSPassSem, 0);

    ticket ticket;
    ticket.nodeID = nodeID;
    ticket.requestID = 0;
    sharedMemoryPointer->competitorTicket = ticket;

    sharedMemoryPointer->maxRequestID = 0;
    sharedMemoryPointer->pendingRequestsCount = 0;
    sharedMemoryPointer->pendingProcessesCount = 0;

    sharedMemoryPointer->hasProcesses = false;

    return sharedMemoryPointer;
}

sharedMemory *getSharedMemory(int nodeID) {
    int key = SHM_KEY + nodeID;
    int id = shmget(key, sizeof(sharedMemory), IPC_CREAT | 0666);
    if (id < 0) {
        printf("shmget error\n");
        exit(1);
    }
    return (sharedMemory *) shmat(id, NULL, 0);
}