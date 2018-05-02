#include <stdio.h>
#include <stdlib.h>
#include "headers/coms.h"
#include "headers/inits.h"
#include "headers/launcherUtils.h"
#include "headers/ticketUtils.h"
#include "headers/priorityUtils.h"

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
        sem_wait(&sharedMemoryPointer->nodeStatusSem);
        updateRequestID(originTicket.requestID);
        char ticketString[100];
        ticketToString(ticketString, originTicket);
        if(!nodeHasProcesses(sharedMemoryPointer)) {
            sendReply(originTicket, nodeID);
        }
        else if (!sharedMemoryPointer->inSC && (compTickets(originTicket, sharedMemoryPointer->competitorTicket) == -1)) { // originTicket < competitorTicket?
            sendReply(originTicket, nodeID);
            sharedMemoryPointer->competitorTicket.priority = NONE;
        }
        else if (sharedMemoryPointer->consultorsInSC > 0) {
            sendReply(originTicket, nodeID);
        }
        else{
            saveRequest(originTicket);
        }
        sem_post(&sharedMemoryPointer->nodeStatusSem);
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
    if(originRequestID >= sharedMemoryPointer->maxRequestID){
        sharedMemoryPointer->maxRequestID = originRequestID + 1;
    }
}

void saveRequest (ticket ticket){
    sharedMemoryPointer->pendingRequests[sharedMemoryPointer->pendingRequestsCount] = ticket;
    sharedMemoryPointer->pendingRequestsCount++;
}

#pragma clang diagnostic pop