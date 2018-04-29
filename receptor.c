#include <stdio.h>
#include <stdlib.h>
#include "headers/coms.h"
#include "headers/inits.h"
#include "headers/launcherUtils.h"
#include "headers/ticketUtils.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

sharedMemory *sharedMemoryPointer;

char receptorTag[100];
int nodeID;

void updateRequestID(int originRequestID);

void saveRequest (ticket ticket);

void initReceptor(int argc, char *argv[]);

void printWrongUsageError();

int main(int argc, char *argv[]){
    initReceptor(argc, argv);
    while(1) {
        ticket originTicket = receiveRequest(nodeID);
        updateRequestID(sharedMemoryPointer->maxRequestID);
        sem_wait(&sharedMemoryPointer->nodeStatusSem);
        if(!sharedMemoryPointer->hasProcesses ||
           (sharedMemoryPointer->hasProcesses && (compTickets(sharedMemoryPointer->competitorTicket, originTicket) == 1))) { // competitorTicket > originTicket?
            sem_post(&sharedMemoryPointer->nodeStatusSem);
            printf("%sEnviando reply a %d\n", receptorTag, originTicket.pid);
            sendReply(originTicket);
        } else{
            sem_post(&sharedMemoryPointer->nodeStatusSem);
            printf("%sGuardando request de %d\n", receptorTag, originTicket.pid);
            saveRequest(originTicket);
        }
    }
}

void initReceptor(int argc, char *argv[]) {
    if (argc != 2) {
        printWrongUsageError();
        exit(0);
    }
    nodeID = atoi(argv[1]);
    sprintf(receptorTag, "R%d> ", nodeID);
    initMailBoxes(nodeID);
    sharedMemoryPointer = initSharedMemory(nodeID);
    sendReceptorConfirmation();
}

void printWrongUsageError() {
    printf("Wrong arguments\nUsage: ./Receptor nodeID\n");
}

void updateRequestID(int originRequestID){
    sem_wait(&sharedMemoryPointer->competitorTicketSem);
    if(originRequestID >= sharedMemoryPointer->maxRequestID){
        sharedMemoryPointer->maxRequestID = originRequestID + 1;
    }
    sem_post(&sharedMemoryPointer->competitorTicketSem);
}

void saveRequest (ticket ticket){
    sharedMemoryPointer->pendingRequestsArray[sharedMemoryPointer->pendingRequestsCount] = ticket;
    sharedMemoryPointer->pendingRequestsCount++;
}

#pragma clang diagnostic pop