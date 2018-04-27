#include <semaphore.h>
#include "../headers/ticketUtils.h"

int compTickets(ticket ticket1, ticket ticket2) {
    if (ticket1.requestID > ticket2.requestID) {
        return 1;
    }
    if (ticket1.requestID < ticket2.requestID) {
        return -1;
    }
    if (ticket1.requestID == ticket2.requestID) {
        if (ticket1.nodeID > ticket2.nodeID) {
            return 1;
        }
        if (ticket1.nodeID < ticket2.nodeID) {
            return -1;
        }
        return 0;
    }
    return 0;
}

ticket createTicket (int nodeID, int *maxPetition, sem_t *semaphore){
    sem_wait(semaphore);
    *maxPetition=(*maxPetition) + 1;
    ticket myTicket = {.nodeID = nodeID, .requestID = *maxPetition};
    sem_post(semaphore);
    return myTicket;
}
