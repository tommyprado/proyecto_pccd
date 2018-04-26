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
#include "headers/coms.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

#define PENDING_REQUESTS_LIMIT 1000000
#define NODE_INITIAL_KEY 10000

void setWantTo (int value);

void accessCS (int type);

void replyAllPending ();

void initNode(int argc, char *argv[]);

void printWrongUsageError();

void updateMaxPetitionID (int petitionId);

void saveRequest (ticket ticket);

int totalNodes, maxPetition, wantTo, nodeID, msgQueue;
sem_t semMaxPetition, semWantTo, semPending;
ticket pendingRequestsArray[PENDING_REQUESTS_LIMIT];
int pendingRequestsCount;
ticket biggestTicket;

int main(int argc, char *argv[]){
    initNode(argc, argv);
    msgQueue = NODE_INITIAL_KEY + nodeID;
    while (1) {
        setWantTo(1);
        ticket ticket = createTicket(nodeID, &maxPetition, &semMaxPetition);
        sendRequests(ticket, nodeID, totalNodes);
        int countReply = 0;
        while (countReply < totalNodes) {
            receiveReply(nodeID);
            countReply++;
        }

        accessCS(0);

        setWantTo(0);
        replyAllPending();
    }
}

void * receptorMain(void *arg) {
    while(1) {
        ticket originTicket = receiveRequest(nodeID);
        updateMaxPetitionID(maxPetition);
        sem_wait(&semWantTo);

        if(wantTo && (compTickets(biggestTicket, originTicket) == 1) ) {
            sem_post(&semWantTo);
            sendReply(biggestTicket);
        }else{
            saveRequest(biggestTicket);
            sem_post(&semWantTo);
        }
    }
}

void updateMaxPetitionID (int petitionId){
    sem_wait(&semMaxPetition);

    if(petitionId>maxPetition){
        maxPetition=petitionId;
    }
    sem_post(&semMaxPetition);
}

void saveRequest (ticket ticket){
    sem_wait(&semPending);
    pendingRequestsArray[pendingRequestsCount] = ticket;
    pendingRequestsCount++;
    sem_post(&semPending);
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
            int msg = msgsnd(msgQueue, &message, sizeof(ticket), 0);
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
    //Enviar reply a todos los nodos de pendingRequestsArray hasta pendingRequestCount
    sem_wait(&semPending);
    for(int i=0;i<pendingRequestsCount;i++){
        if( (msgsnd(msgQueue, &pendingRequestsArray[i], sizeof(pendingRequestsArray[i])-sizeof(long), 0)) == -1) {
            printf("Error al invocar 'msgrcv()'.\n");
            exit(0);
        } else {
            printf("Ticket nº %i retirado correctamente.\n", &i);
        }
    }
    sem_post(&semPending);

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
