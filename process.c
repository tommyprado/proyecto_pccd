#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <zconf.h>
#include "headers/ticketUtils.h"
#include "headers/inits.h"
#include "headers/coms.h"
#include "headers/launcherUtils.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

#define PENDING_REQUESTS_LIMIT 1000000
#define NODE_REQUEST_BASE 10000

#define SC_WAIT 1

void setWantTo (int value);

void initNode(int argc, char *argv[], int pid);

void printWrongUsageError();

void updateMaxPetition(int petitionId);

void saveRequest (ticket ticket);

void accessCS (ticket ticket);

sharedMemStruct *sharedMemoryPointer;

int totalNodes, nodeID;

char processTag[100];

int main(int argc, char *argv[]){
    int pid = getpid();
    initNode(argc, argv, pid);
    printf("%sIntentando acceder a la sección crítica...\n", processTag);
    setWantTo(1);
    sem_wait(&sharedMemoryPointer->semTicket);
    sharedMemoryPointer->myTicket = createTicket(nodeID, pid, &sharedMemoryPointer->maxPetition,
                                                 &sharedMemoryPointer->semMaxPetition);
    sendRequests(sharedMemoryPointer->myTicket, nodeID, totalNodes);
    sem_post(&sharedMemoryPointer->semTicket);
    int countReply = 1;
    while (countReply < totalNodes) {
        receiveReply(nodeID, pid);
        printf("%sReply recibido\n", processTag);
        countReply++;
    }
    printf("%sAccediendo a la sección crítica...\n", processTag);
    accessCS(sharedMemoryPointer->myTicket);
    printf("%sFuera de la sección crítica\n", processTag);
    setWantTo(0);
    printf("%sRespondiendo a Requests pendientes...\n", processTag);
    replyAllPending(&sharedMemoryPointer->semPending, &sharedMemoryPointer->pendingRequestsCount, sharedMemoryPointer->pendingRequestsArray, nodeID);
}

void setWantTo (int value){
    sem_wait(&sharedMemoryPointer->semWantTo);
    sharedMemoryPointer->wantTo=value;
    sem_post(&sharedMemoryPointer->semWantTo);
}

void accessCS (ticket ticket){
    sndMsgOut(TYPE_ACCESS_CS, ticket);
    printf("%sEn sección crítica\n", processTag);
    sleep(1);
    printf("%sSaliendo de sección crítica\n", processTag);
    sndMsgOut(TYPE_EXIT_CS, ticket);
}

void initNode(int argc, char *argv[], int pid) {
    if (argc != 3) {
        printWrongUsageError();
        exit(0);
    }
    nodeID = atoi(argv[1]);
    totalNodes = atoi(argv[2]);
    if (nodeID > totalNodes) {
        printWrongUsageError();
    }
    sprintf(processTag, "N%d %d> ", nodeID, pid);

    sharedMemoryPointer = getSharedMemory(nodeID);
}

void printWrongUsageError() {
    printf("Wrong arguments\nUsage: ./Process nodeID totalNodes mode (nodeID <= totalNodes)\n");
}

#pragma clang diagnostic pop
