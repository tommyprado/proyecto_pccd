#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <zconf.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <pthread.h>
#include <sys/time.h>
#include "headers/ticketUtils.h"
#include "headers/inits.h"
#include "headers/coms.h"
#include "headers/out.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

#define PENDING_REQUESTS_LIMIT 1000000
#define NODE_INITIAL_KEY 10000

#define SC_WAIT 10000

void setWantTo (int value);

void initNode(int argc, char *argv[]);

void printWrongUsageError();

void updateMaxPetition(int petitionId);

void saveRequest (ticket ticket);

void accessCS (ticket ticket);

sharedMemStruct *sharedMemoryPointer;

int totalNodes, nodeID;

char mainTag[100];

int main(int argc, char *argv[]){
    initNode(argc, argv);
    printf("%sIntentando acceder a la sección crítica...\n", mainTag);
    setWantTo(1);
    sem_wait(&sharedMemoryPointer->semTicket);
    sharedMemoryPointer->myTicket = createTicket(nodeID, &sharedMemoryPointer->maxPetition, &sharedMemoryPointer->semMaxPetition);
    sendRequests(sharedMemoryPointer->myTicket, nodeID, totalNodes);
    sem_post(&sharedMemoryPointer->semTicket);
    int countReply = 1;
    while (countReply < totalNodes) {
        receiveReply(nodeID);
        printf("%sReply recibido\n", mainTag);
        countReply++;
    }
    printf("%sAccediendo a la sección crítica...\n", mainTag);
    accessCS(sharedMemoryPointer->myTicket);
    printf("%sFuera de la sección crítica\n", mainTag);
    setWantTo(0);
    printf("%sRespondiendo a Requests pendientes...\n", mainTag);
    replyAllPending(&sharedMemoryPointer->semPending, &sharedMemoryPointer->pendingRequestsCount, sharedMemoryPointer->pendingRequestsArray, nodeID);
}

void setWantTo (int value){
    sem_wait(&sharedMemoryPointer->semWantTo);
    sharedMemoryPointer->wantTo=value;
    sem_post(&sharedMemoryPointer->semWantTo);
}

void accessCS (ticket ticket){
//    sndMsgOut(TYPE_ENTRO, ticket);
    usleep(SC_WAIT * 1000);
//    sndMsgOut(TYPE_SALGO, ticket);
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
    sprintf(mainTag, "MAIN %d> ", nodeID);

    sharedMemoryPointer = getSharedMemory(nodeID);
}

void printWrongUsageError() {
    printf("Wrong arguments\nUsage: ./Process nodeID totalNodes mode (nodeID <= totalNodes)\n");
}

#pragma clang diagnostic pop
