#include "ticketUtils.h"
#include "inits.h"

#ifndef PROYECTO_COMS_H
#define PROYECTO_COMS_H

#define NODE_REQUEST_BASE 100000
#define NODE_REPLY_BASE 200000

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

void sendRequests(ticket ticket, int totalNodes);

void receiveReply(int nodeID, int i);

void sendReply (ticket ticket);

void replyAllPending (sharedMemory *sharedMemory);

int getNodeReplyMsqid(int nodeID);

int getNodeRequestMsqid(int nodeID);

int getMsqid(int key);

#endif //PROYECTO_COMS_H
