#ifndef PROYECTO_TICKETUTILS_H
#define PROYECTO_TICKETUTILS_H

#include <semaphore.h>

#define PAGOS 0
#define ANULACIONES 1
#define RESERVAS 2
#define CONSULTORES 3
#define NONE 4

typedef struct
{
    int nodeID;
    int requestID;
    int priority;
    int pid;
} ticket;

int compTickets(ticket ticket1, ticket ticket2);

void ticketToString(char *string, ticket ticket);

#endif //PROYECTO_TICKETUTILS_H
