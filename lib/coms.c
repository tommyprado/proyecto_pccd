#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include "../headers/coms.h"
#include "../headers/ticketUtils.h"

#define TYPE_REQUEST 1
#define TYPE_REPLY 2

ticket receiveRequest (int nodeID) {
    ticketMessage message;
    int msqid = getNodeRequestMsqid(nodeID);
    msgrcv(msqid, &message, sizeof(ticket), TYPE_REQUEST, 0);
    return message.ticket;
}

void sendRequests(ticket ticket, int nodeID, int totalNodes){
    for(int node = 1; node < totalNodes + 1; node++){
        if(node != nodeID){
            int msqid = getNodeRequestMsqid(node);
            ticketMessage message;
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

void sendReply (ticket ticket){
    int msqid = getNodeReplyMsqid(ticket.nodeID);
    ticketMessage message;
    message.mtype = ticket.pid;
    message.ticket = ticket;
    int msg = msgsnd(msqid, &message, sizeof(ticket), 0);
    if(msg == -1) {
        printf("Error al enviar el ticket\n");
        exit(1);
    }
}

void replyAllPending (sem_t *semPending, int *pendingRequestsCount, ticket * pendingRequestsArray, int nodeID){
    sem_wait(semPending);
    for(int i=0; i < *pendingRequestsCount; i++){
        sendReply(pendingRequestsArray[i]);
    }
    *pendingRequestsCount = 0;
    sem_post(semPending);
}

void receiveReply(int nodeID, int pid) {
    int msqid = getNodeReplyMsqid(nodeID);
    ticketMessage message;
    msgrcv(msqid, &message, sizeof(ticket), pid, 0);
}

int getNodeReplyMsqid(int nodeID) {
    int key= nodeID + NODE_REPLY_BASE;
    int msqid = msgget(key, 0666);
    if (msqid == -1)
    {
        printf("Error recuperando buzón\n");
        exit (-1);
    }
    return msqid;
}

int getNodeRequestMsqid(int nodeID) {
    int key= nodeID + NODE_REQUEST_BASE;
    int msqid = msgget(key, 0666);
    if (msqid == -1)
    {
        printf("Error recuperando buzón\n");
        exit (-1);
    }
    return msqid;
}


int getMsqid(int key) {
    int msqid = msgget(key, 0666);
    if (msqid == -1)
    {
        printf("Error recuperando buzón\n");
        exit (-1);
    }
    return msqid;
}


