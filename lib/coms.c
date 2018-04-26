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
    messageBuff message;
    message.mtype = TYPE_REPLY;
    message.ticket = ticket;
    int msg = msgsnd(ticket.nodeID+NODE_INITIAL_KEY, &message, sizeof(ticket), 0);
    if(msg == -1) {
        printf("Error al enviar el ticket\n");
        exit(1);
    }
}

ticket receiveRequest (int nodeID){
    messageBuff message;
    msgrcv(NODE_INITIAL_KEY + nodeID, &message, sizeof(ticket), TYPE_REQUEST, 0);
}

void sendRequests(ticket ticket, int nodeID, int totalNodes){
    for(int node = 1; node < totalNodes; node++){
        printf("Enviamos el ticket al nodo %i\n", node);
        if(!node == nodeID){
            messageBuff message;
            message.mtype = TYPE_REQUEST;
            message.ticket = ticket;
            int msg = msgsnd(NODE_INITIAL_KEY + node, &message, sizeof(ticket), 0);
            if(msg == -1) {
                printf("Error al enviar el ticket\n");
                exit(1);
            }
        }
    }
}

void receiveReply (int nodeID){
    messageBuff message;
    msgrcv(NODE_INITIAL_KEY + nodeID, &message, sizeof(ticket), TYPE_REPLY, 0);
}
