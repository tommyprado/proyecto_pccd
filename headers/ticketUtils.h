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

void ticketToString(char *string, ticket ticket);

#endif //PROYECTO_TICKETUTILS_H
