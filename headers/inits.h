#ifndef PROYECTO_INITS_H
#define PROYECTO_INITS_H

#include "ticketUtils.h"
#define PENDING_REQUESTS_LIMIT 1000000

typedef struct {
    int maxPetition, wantTo, pendingRequestsCount;
    sem_t semMaxPetition, semWantTo, semPending, semTicket;
    ticket myTicket;
    ticket pendingRequestsArray[PENDING_REQUESTS_LIMIT];
} sharedMemStruct;

void initMailBoxes(int nodeID);

void initSemaphore(sem_t *semaphore);

sharedMemStruct *getSharedMemory(int nodeID);

sharedMemStruct *initSharedMemory(int nodeID);

#endif //PROYECTO_INITS_H
