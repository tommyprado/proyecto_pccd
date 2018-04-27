#include "ticketUtils.h"

#ifndef PROYECTO_COMS_H
#define PROYECTO_COMS_H

ticket receiveRequest (int nodeID);

void sendRequests(ticket ticket, int nodeID, int totalNodes);

void receiveReply (int nodeID);

void sendReply (ticket ticket);

void replyAllPending (sem_t *semPending, int *pendingRequestsCount, ticket * pendingRequestsArray, int nodeID);

int getMsqid(int i);

#endif //PROYECTO_COMS_H
