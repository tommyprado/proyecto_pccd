#ifndef PROYECTO_INITS_H
#define PROYECTO_INITS_H

#include <stdbool.h>
#include "ticketUtils.h"
#define PENDING_REQUESTS_LIMIT 10000
#define SHM_KEY 20000

typedef struct {
    int maxRequestID;
    sem_t nodeStatusSem;
    ticket competitorTicket;

    int nextPagosCount, nextAnulacionesCount, nextReservasCount, nextConsultoresCount;
    sem_t nextPagosSem, nextAnulacionesSem, nextReservasSem, nextConsultoresSem;

    int pendingRequestsCount;
    ticket pendingRequests[PENDING_REQUESTS_LIMIT];
    bool inSC;
} sharedMemory;

void initMailBoxes(int nodeID);

void initSemaphore(sem_t *semaphore, unsigned int i);

sharedMemory *getSharedMemory(int nodeID);

sharedMemory *initSharedMemory(int nodeID);

void sendRequest (ticket ticket, int node) ;

#endif //PROYECTO_INITS_H
