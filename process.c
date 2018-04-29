#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <zconf.h>
#include <stdbool.h>
#include "headers/ticketUtils.h"
#include "headers/inits.h"
#include "headers/coms.h"
#include "headers/launcherUtils.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

#define PENDING_REQUESTS_LIMIT 1000000
#define NODE_REQUEST_BASE 10000

#define SC_WAIT 3

void initNode(int argc, char *argv[]);

void printWrongUsageError();

void updateRequestID(int originRequestID);

void saveRequest (ticket ticket);

void accessCS (ticket ticket);

void waitForCSAccess();

void competeForCSAccess() ;

void createCompetitorTicket();

sharedMemory *sharedMemoryPointer;

int totalNodes, nodeID, pid;

char processTag[100];

int main(int argc, char *argv[]){
    initNode(argc, argv);
    sem_wait(&sharedMemoryPointer->nodeStatusSem);
    if (!sharedMemoryPointer->hasProcesses) {
        printf("%sPrimer proceso\n", processTag);
        sharedMemoryPointer->hasProcesses = true;
        createCompetitorTicket();
        sem_post(&sharedMemoryPointer->nodeStatusSem);
        competeForCSAccess();
    } else {
        sem_post(&sharedMemoryPointer->nodeStatusSem);
        waitForCSAccess();
    }

    accessCS(sharedMemoryPointer->competitorTicket);

    sem_wait(&sharedMemoryPointer->nodeStatusSem);
    if (sharedMemoryPointer->pendingProcessesCount != 0) {
        sharedMemoryPointer->pendingProcessesCount = sharedMemoryPointer->pendingProcessesCount - 1;
        sem_post(&sharedMemoryPointer->allowNextCSPassSem);
    } else {
        printf("%sRespondiendo a Requests pendientes...\n", processTag);
        sharedMemoryPointer->competitorTicket.requestID = -1;
        sharedMemoryPointer->hasProcesses = false;
        replyAllPending(sharedMemoryPointer);
    }
    sem_post(&sharedMemoryPointer->nodeStatusSem);
}

void createCompetitorTicket() {
    sem_wait(&sharedMemoryPointer->competitorTicketSem);
    sharedMemoryPointer->maxRequestID = sharedMemoryPointer->maxRequestID + 1;
    ticket ticket = {.nodeID = nodeID, .requestID = sharedMemoryPointer->maxRequestID, .pid=pid};
    sharedMemoryPointer->competitorTicket = ticket;
    sem_post(&sharedMemoryPointer->competitorTicketSem);
}

void waitForCSAccess() {
    sharedMemoryPointer->pendingProcessesCount = sharedMemoryPointer->pendingProcessesCount + 1;
    sem_wait(&sharedMemoryPointer->allowNextCSPassSem);
}

void competeForCSAccess() {
    sendRequests(sharedMemoryPointer->competitorTicket, totalNodes);
    int countReply = 1;
    while (countReply < totalNodes) {
        receiveReply(nodeID, pid);
        countReply++;
    }
}

void accessCS (ticket ticket){
    sndMsgOut(TYPE_ACCESS_CS, ticket);
    printf("%sEn sección crítica\n", processTag);
    sleep(SC_WAIT);
    printf("%sSaliendo de sección crítica\n", processTag);
    sndMsgOut(TYPE_EXIT_CS, ticket);
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
    pid = getpid();
    sprintf(processTag, "N%d %d> ", nodeID, pid);

    sharedMemoryPointer = getSharedMemory(nodeID);
}

void printWrongUsageError() {
    printf("Wrong arguments\nUsage: ./Process nodeID totalNodes mode (nodeID <= totalNodes)\n");
}

#pragma clang diagnostic pop
