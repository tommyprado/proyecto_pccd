#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <zconf.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <pthread.h>
#include "headers/ticketUtils.h"
#include "headers/inits.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

#define TYPE_REQUEST 1
#define TYPE_REPLY 2
#define PENDING_REQUESTS_LIMIT 1000000
#define NODE_INITIAL_KEY 10000


typedef struct {
    long    mtype;
    ticket  ticket;
} messageBuff;

void setWantTo (int value);

void doStuff (int type);

void sendRequests(ticket ticket);

void receiveReply ();

void accessCS (int type);

void replyAllPending ();

void initNode(int argc, char *argv[]);

void printWrongUsageError();

ticket receiveRequest ();

void updateMaxPetitionID (int petitionId);

void protectWantTo ();

void unprotectWantTo ();

void sendReply (ticket ticket);

void saveRequest (ticket ticket);

int totalNodes, maxPetition, wantTo, nodeID;
sem_t semMaxPetition, semWantTo, semPending;
ticket pendingRequestsArray[PENDING_REQUESTS_LIMIT];
int pendingRequestsCount;
ticket biggestTicket;

int main(int argc, char *argv[]){
    initNode(argc, argv);
    while (1) {
        doStuff(0);

        setWantTo(1);
        ticket ticket = createTicket(nodeID, &maxPetition, &semMaxPetition);
        sendRequests(ticket);
        int countReply = 0;
        while (countReply < totalNodes) {
            receiveReply();
            countReply++;
        }

        accessCS(0);

        setWantTo(0);
        replyAllPending();
    }
}

void * receptorMain(void *arg) {
    while(1) {
        ticket originTicket = receiveRequest();
        updateMaxPetitionID(maxPetition);
        protectWantTo();

        if(wantTo && (compTickets(biggestTicket, originTicket) == 1) ) {
            unprotectWantTo();
            sendReply(biggestTicket);
        }else{
            saveRequest(biggestTicket);
            unprotectWantTo();
        }
    }
}

ticket receiveRequest (){
    messageBuff message;
    msgrcv(NODE_INITIAL_KEY + nodeID, &message, sizeof(ticket), TYPE_REQUEST, 0);
}

void updateMaxPetitionID (int petitionId){
    sem_wait(&semMaxPetition);

    if(petitionId>maxPetition){
        maxPetition=petitionId;
    }
    sem_post(&semMaxPetition);
}

void protectWantTo (){
    sem_wait(&semWantTo);
}

void saveRequest (ticket ticket){
    sem_wait(&semPending);
    pendingRequestsArray[pendingRequestsCount] = ticket;
    pendingRequestsCount++;
    sem_post(&semPending);
}

void unprotectWantTo (){
    sem_post(&semWantTo);
}

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

void doStuff (int type){
    if(type == 0){
        printf("\nEsperando salto de linea...\n");
        getchar();
        return;
    }
    usleep(100*1000);
}

void setWantTo (int value){
    sem_wait(&semWantTo);
    wantTo=value;
    sem_post(&semWantTo);
}

void sendRequests(ticket ticket){
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

void receiveReply (){
    messageBuff message;
    msgrcv(NODE_INITIAL_KEY + nodeID, &message, sizeof(ticket), TYPE_REPLY, 0);
}

void accessCS (int type){
    if(type == 0){
        printf("\nEsperando salto de linea...\n");
        getchar();
        return;
    }
    usleep(100*1000);
}

void replyAllPending (){
    // TODO: replyAllPending
}

void initNode(int argc, char *argv[]) {
    if (argc != 3) {
        printWrongUsageError();
        exit(0);
    }
    nodeID = atoi(argv[1]);
    totalNodes = atoi(argv[2]);
    if (nodeID > totalNodes) {
        printWrongUsageError();
    }

    initSemaphores(&semMaxPetition, &semWantTo, &semPending);
    initMailBoxes(nodeID);
    initReceptor(receptorMain);
}

void printWrongUsageError() {
    printf("Wrong arguments\nUsage: ./main nodeID totalNodes (nodeID <= totalNodes)");
}

#pragma clang diagnostic pop
