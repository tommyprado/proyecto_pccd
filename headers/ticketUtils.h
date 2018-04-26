#ifndef PROYECTO_TICKETUTILS_H
#define PROYECTO_TICKETUTILS_H

#include <semaphore.h>

typedef struct
{
    int nodeID;
    int requestID;

} ticket;

int compTickets(ticket ticket1, ticket ticket2);

ticket createTicket (int nodeID, int *maxPetition, sem_t *semaphore);

#endif //PROYECTO_TICKETUTILS_H
