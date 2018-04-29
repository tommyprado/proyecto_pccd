#ifndef PROYECTO_INITS_H
#define PROYECTO_INITS_H

#include <stdbool.h>
#include "ticketUtils.h"
#define PENDING_REQUESTS_LIMIT 1000000

typedef struct {
    int maxRequestID, pendingRequestsCount, pendingProcessesCount;
    sem_t competitorTicketSem, nodeStatusSem, allowNextCSPassSem;
    ticket competitorTicket;
    ticket pendingRequestsArray[PENDING_REQUESTS_LIMIT];
    bool hasProcesses;
} sharedMemory;

void initMailBoxes(int nodeID);

void initSemaphore(sem_t *semaphore, int i);

sharedMemory *getSharedMemory(int nodeID);

sharedMemory *initSharedMemory(int nodeID);

#endif //PROYECTO_INITS_H
