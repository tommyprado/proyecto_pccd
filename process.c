#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>
#include "headers/ticketUtils.h"
#include "headers/inits.h"
#include "headers/coms.h"
#include "headers/launcherUtils.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

#define PENDING_REQUESTS_LIMIT 1000000

#define SC_WAIT 500

void initNode(int argc, char *argv[]);

void printWrongUsageError();

void updateRequestID(int originRequestID);

void saveRequest (ticket ticket);

void accessCS (ticket ticket);

void waitForCSAccess();

void createCompetitorTicket();

sharedMemory *sharedMemoryPointer;

int totalNodes, nodeID, pid;

char processTag[100];

char stringTicket[10];

int main(int argc, char *argv[]){
    initNode(argc, argv);
    sem_wait(&sharedMemoryPointer->nodeStatusSem);
    if (!sharedMemoryPointer->hasProcesses) {
        sharedMemoryPointer->hasProcesses = true;
        createCompetitorTicket();
        printf("%sPrimer proceso.\n", processTag);
        sendRequests(sharedMemoryPointer->competitorTicket, totalNodes);
        sem_post(&sharedMemoryPointer->nodeStatusSem);
        int countReply = 0;
        while (countReply < totalNodes - 1) {
            countReply++;
            long node = receiveReply(nodeID, pid);
            //printf("%sRecibido reply from %ld\n", processTag, node);
        }
    } else {
        sem_post(&sharedMemoryPointer->nodeStatusSem);
        waitForCSAccess();
    }

    accessCS(sharedMemoryPointer->competitorTicket);

    sem_wait(&sharedMemoryPointer->nodeStatusSem);
    if (sharedMemoryPointer->pendingProcessesCount != 0) {
        //printf("%sWaking up other\n", processTag);
        sharedMemoryPointer->pendingProcessesCount = sharedMemoryPointer->pendingProcessesCount - 1;
        sem_post(&sharedMemoryPointer->allowNextCSPassSem);
    } else {
        sharedMemoryPointer->competitorTicket.requestID = -1;
        sharedMemoryPointer->hasProcesses = false;
        replyAllPending(sharedMemoryPointer, nodeID);
    }
    sem_post(&sharedMemoryPointer->nodeStatusSem);
    sndMsgToLauncher(TYPE_PROCESS_FINISHED);
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

void accessCS (ticket ticket){
    sndTicketToLauncher(TYPE_ACCESS_CS, ticket);
    ticketToString(stringTicket, sharedMemoryPointer->competitorTicket);
    printf("%sEn sección crítica con ticket %s\n", processTag, stringTicket);
    usleep(SC_WAIT * 1000);
    printf("%sSaliendo de sección crítica con ticket %s\n", processTag, stringTicket);
    sndTicketToLauncher(TYPE_EXIT_CS, ticket);
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
    sprintf(processTag, "N%d -> ", nodeID);

    sharedMemoryPointer = getSharedMemory(nodeID);
}

void printWrongUsageError() {
    printf("Wrong arguments\nUsage: ./Process nodeID totalNodes mode (nodeID <= totalNodes)\n");
}

#pragma clang diagnostic pop
