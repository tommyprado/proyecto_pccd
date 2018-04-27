#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include "../headers/coms.h"
#include "../headers/ticketUtils.h"

#define TYPE_REQUEST 1
#define TYPE_REPLY 2

#define NODE_INITIAL_KEY 10000

typedef struct {
    long    mtype;
    ticket  ticket;
} messageBuff;

void sendReply (ticket ticket){
    int msqid = getMsqid(ticket.nodeID);
    messageBuff message;
    message.mtype = TYPE_REPLY;
    message.ticket = ticket;
    int msg = msgsnd(msqid, &message, sizeof(ticket), 0);
    if(msg == -1) {
        printf("Error al enviar el ticket\n");
        exit(1);
    }
}

ticket receiveRequest (int nodeID) {
    messageBuff message;
    int msqid = getMsqid(nodeID);
    msgrcv(msqid, &message, sizeof(ticket), TYPE_REQUEST, 0);
    return message.ticket;
}

void sendRequests(ticket ticket, int nodeID, int totalNodes){
    for(int node = 1; node < totalNodes + 1; node++){
        if(node != nodeID){
            int msqid = getMsqid(node);
            messageBuff message;
            message.mtype = TYPE_REQUEST;
            message.ticket = ticket;
            int msg = msgsnd(msqid, &message, sizeof(ticket), 0);
            if(msg == -1) {
                printf("Error al enviar el ticket\n");
                exit(1);
            }
        }
    }
}

void replyAllPending (sem_t *semPending, int *pendingRequestsCount, ticket * pendingRequestsArray, int nodeID){
    sem_wait(semPending);
    for(int i=0; i < *pendingRequestsCount; i++){
        ticket ticket = pendingRequestsArray[i];
        int msqid = getMsqid(ticket.nodeID);
        messageBuff message;
        message.mtype = TYPE_REPLY;
        message.ticket = ticket;
        if(msgsnd(msqid, &message, sizeof(ticket), 0) == -1) {
            printf("Error al invocar 'msgrcv()'.\n");
            exit(0);
        }
    }
    *pendingRequestsCount = 0;
    sem_post(semPending);
}

void receiveReply (int nodeID){
    int msqid = getMsqid(nodeID);
    messageBuff message;
    msgrcv(msqid, &message, sizeof(ticket), TYPE_REPLY, 0);
}

int getMsqid(int nodeID) {
    int key= nodeID + NODE_INITIAL_KEY;
    int msqid = msgget(key, 0666);
    if (msqid == -1)
    {
        printf("Error recuperando buzón\n");
        exit (-1);
    }
    return msqid;
}


