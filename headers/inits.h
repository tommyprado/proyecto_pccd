#ifndef PROYECTO_INITS_H
#define PROYECTO_INITS_H

#include <stdbool.h>
#include "ticketUtils.h"
#define PENDING_REQUESTS_LIMIT 1000000
#define SHM_KEY 20000

typedef struct {
    int maxRequestID, pendingRequestsMaxPriority;
    sem_t competitorTicketSem, nodeStatusSem;
    ticket competitorTicket;
    ticket pagosTicket, anulacionesTicket, reservasTicket, consultoresTicket;

    int nextPagosCount, nextAnulacionesCount, nextReservasCount, nextConsultoresCount;
    sem_t nextPagosSem, nextAnulacionesSem, nextReservasSem, nextConsultoresSem;

    int pendingPagosCount, pendingAnulacionesCount, pendingReservasCount, pendingConsultoresCount;
    ticket pendingPagosArray[PENDING_REQUESTS_LIMIT];
    ticket pendingAnulacionesArray[PENDING_REQUESTS_LIMIT];
    ticket pendingReservasArray[PENDING_REQUESTS_LIMIT];
    ticket pendingConsultoresArray[PENDING_REQUESTS_LIMIT];
} sharedMemory;

void initMailBoxes(int nodeID);

void initSemaphore(sem_t *semaphore, unsigned int i);

sharedMemory *getSharedMemory(int nodeID);

sharedMemory *initSharedMemory(int nodeID);

#endif //PROYECTO_INITS_H
