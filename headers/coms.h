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
    int origin;
} ticketMessage;

ticket receiveRequest (int nodeID);

void sendRequests(ticket ticket, int totalNodes);

int receiveReply(ticket ticket);

void sendReply (ticket ticket, int originID);

int getNodeReplyMsqid(int nodeID);

int getNodeRequestMsqid(int nodeID);

int getMsqid(int key);

#endif //PROYECTO_COMS_H
