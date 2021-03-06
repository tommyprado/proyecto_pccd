#include "../headers/coms.h"
#include "../headers/ticketUtils.h"
#include "../headers/inits.h"
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>

ticket receiveRequest (int nodeID) {
    ticketMessage message;
    int msqid = getNodeRequestMsqid(nodeID);
    msgrcv(msqid, &message, sizeof(message) - sizeof(long), 0, 0);
    return message.ticket;
}

void sendRequests(ticket ticket, int totalNodes){
    for(int node = 1; node < totalNodes + 1; node++){
        if(node != ticket.nodeID){
            sendRequest(ticket, node);
        }
    }
}

void sendRequest (ticket ticket, int node) {
    int msqid = getNodeRequestMsqid(node);
    ticketMessage message;
    message.ticket = ticket;
    message.mtype = ticket.pid;
    int msg = msgsnd(msqid, &message, sizeof(ticketMessage) - sizeof(long), 0);
    if(msg == -1) {
        perror("Error al enviar el ticket");
        exit(1);
    }
}

void sendReply (ticket ticket, int originID){
    int msqid = getNodeReplyMsqid(ticket.nodeID);
    ticketMessage message;
    message.mtype = ticket.pid;
    message.ticket = ticket;
    message.origin = originID;
    int msg = msgsnd(msqid, &message, sizeof(ticketMessage) - sizeof(long), 0);
    if(msg == -1) {
        perror("Error al enviar el reply");
        exit(1);
    }
}

void receiveReply(ticket ticket, ticketMessage *message) {
    int msqid = getNodeReplyMsqid(ticket.nodeID);
    msgrcv(msqid, message, sizeof(ticketMessage) - sizeof(long), ticket.pid, 0);
}

int getNodeReplyMsqid(int nodeID) {
    int key= nodeID + NODE_REPLY_BASE;
    int msqid = msgget(key, 0666);
    if (msqid == -1)
    {
        perror("Error recuperando buzón");
        exit (-1);
    }
    return msqid;
}

int getNodeRequestMsqid(int nodeID) {
    int key= nodeID + NODE_REQUEST_BASE;
    int msqid = msgget(key, 0666);
    if (msqid == -1)
    {
        perror("Error recuperando buzón");
        exit (-1);
    }
    return msqid;
}


int getMsqid(int key) {
    int msqid = msgget(key, 0666);
    if (msqid == -1)
    {
        perror("Error recuperando buzón");
        exit (-1);
    }
    return msqid;
}


