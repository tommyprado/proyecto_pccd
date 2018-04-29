#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include "../headers/coms.h"
#include "../headers/ticketUtils.h"
#include "../headers/inits.h"

ticket receiveRequest (int nodeID) {
    ticketMessage message;
    int msqid = getNodeRequestMsqid(nodeID);
    msgrcv(msqid, &message, sizeof(message) - sizeof(long), 0, 0);
    return message.ticket;
}

void sendRequests(ticket ticket, int totalNodes){
    for(int node = 1; node < totalNodes + 1; node++){
        if(node != ticket.nodeID){
            int msqid = getNodeRequestMsqid(node);
            ticketMessage message;
            message.mtype = ticket.pid;
            message.ticket = ticket;
            int msg = msgsnd(msqid, &message, sizeof(ticketMessage) - sizeof(long), 0);
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
    int msg = msgsnd(msqid, &message, sizeof(ticketMessage) - sizeof(long), 0);
    if(msg == -1) {
        printf("Error al enviar el ticket\n");
        exit(1);
    }
}

void replyAllPending (sharedMemory *sharedMemory){
    for(int i=0; i < sharedMemory->pendingRequestsCount; i++){
        sendReply(sharedMemory->pendingRequestsArray[i]);
    }
    sharedMemory->pendingRequestsCount = 0;
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


