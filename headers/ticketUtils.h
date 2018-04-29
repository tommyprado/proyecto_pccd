#ifndef PROYECTO_TICKETUTILS_H
#define PROYECTO_TICKETUTILS_H

#include <semaphore.h>

typedef struct
{
    int nodeID;
    int requestID;
    int pid;
} ticket;

int compTickets(ticket ticket1, ticket ticket2);

ticket createTicket(int nodeID, int maxPetition, int *semaphore, sem_t *ptr);

#endif //PROYECTO_TICKETUTILS_H
