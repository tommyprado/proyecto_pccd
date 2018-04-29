#include "ticketUtils.h"

#ifndef PROYECTO_COMS_H
#define PROYECTO_COMS_H

#define NODE_REQUEST_BASE 10000
#define NODE_REPLY_BASE 20000

typedef struct {
    long    mtype;
    ticket  ticket;
    long long int t;
} launcherMessage;

typedef struct {
    long    mtype;
    ticket  ticket;
} ticketMessage;

ticket receiveRequest (int nodeID);

void sendRequests(ticket ticket, int nodeID, int totalNodes);

void receiveReply(int nodeID, int i);

void sendReply (ticket ticket);

void replyAllPending (sem_t *semPending, int *pendingRequestsCount, ticket * pendingRequestsArray, int nodeID);

int getNodeReplyMsqid(int nodeID);

int getNodeRequestMsqid(int nodeID);

int getMsqid(int key);

#endif //PROYECTO_COMS_H
