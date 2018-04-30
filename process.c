#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>
#include "headers/ticketUtils.h"
#include "headers/inits.h"
#include "headers/coms.h"
#include "headers/launcherUtils.h"
#include "headers/priorityUtils.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

#define PENDING_REQUESTS_LIMIT 1000000

#define SC_WAIT 500

void initNode(int argc, char *argv[]);

void printWrongUsageError();

void updateRequestID(int originRequestID);

void saveRequest (ticket ticket);

void accessCS (ticket ticket);

ticket createTicket();

void replyPendingRequests(ticket ticket);

void wakeNextInLine();

sharedMemory *sharedMemoryPointer;

int totalNodes, nodeID, pid, priority;

char processTag[100];

int main(int argc, char *argv[]){
    initNode(argc, argv);

    while (1) {
        sem_wait(&sharedMemoryPointer->nodeStatusSem);
        if (!nodeHasProcesses(sharedMemoryPointer)) {
            addProcessToCount(sharedMemoryPointer, priority);
            postByPriority(sharedMemoryPointer, priority);
            sem_post(&sharedMemoryPointer->nodeStatusSem);
            break;
        }

        if (priority >= sharedMemoryPointer->competitorTicket.priority) {
            sem_post(&sharedMemoryPointer->nodeStatusSem);
            waitByPriority(sharedMemoryPointer, priority);
            sem_wait(&sharedMemoryPointer->nodeStatusSem);
        }

        ticket ticket = createTicket();
        replyPendingRequests(ticket);
        sendRequests(ticket, totalNodes);
        for (int i = 0; i < totalNodes; ++i) {
            receiveReply(ticket);
            if (compTickets(sharedMemoryPointer->competitorTicket, ticket) != 0) {
                sem_post(&sharedMemoryPointer->nodeStatusSem);
                continue;
            }
        }
        sem_post(&sharedMemoryPointer->nodeStatusSem);
        break;
    }

    accessCS(sharedMemoryPointer->competitorTicket);

    sem_wait(&sharedMemoryPointer->nodeStatusSem);
    removeProcessFromCount(sharedMemoryPointer, priority);
    wakeNextInLine();
    sem_post(&sharedMemoryPointer->nodeStatusSem);

    sndMsgToLauncher(TYPE_PROCESS_FINISHED);
}

void replyPendingRequests(ticket ticket) {
    ticket newPending[PENDING_REQUESTS_LIMIT];
    int newPendingCount = 0;
    for (int i = 0; i < sharedMemoryPointer->pendingRequestsCount; ++i) {
        ticket pendingRequest = sharedMemoryPointer->pendingRequests[i];
        if (compTickets(pendingRequest, ticket) == -1) {
            sendReply(pendingRequest, nodeID);
        } else {
            newPending[newPendingCount] = pendingRequest;
            newPendingCount++;
        }
    }

    for (int j = 0; j < newPendingCount; ++j) {
        sharedMemoryPointer->pendingRequests[j] = newPending[j];
    }
    sharedMemoryPointer->pendingRequestsCount = newPendingCount;
}

void wakeNextInLine() {
    if (priorityHasProcesses(sharedMemoryPointer, PAGOS)) {
        sem_post(&sharedMemoryPointer->nextPagosSem);
    } else if (priorityHasProcesses(sharedMemoryPointer, ANULACIONES)) {
        sem_post(&sharedMemoryPointer->nextAnulacionesSem);
    } else if (priorityHasProcesses(sharedMemoryPointer, RESERVAS)) {
        sem_post(&sharedMemoryPointer->nextReservasSem);
    } else if (priorityHasProcesses(sharedMemoryPointer, CONSULTORES)) {
        sem_post(&sharedMemoryPointer->nextConsultoresSem);
    }
}

ticket createTicket() {
    sharedMemoryPointer->maxRequestID = sharedMemoryPointer->maxRequestID + 1;
    ticket ticket = {.nodeID = nodeID, .requestID = sharedMemoryPointer->maxRequestID, .pid=pid, .priority = priority};
    return ticket;
}

void accessCS (ticket ticket){
    sndTicketToLauncher(TYPE_ACCESS_CS, ticket);
    printf("%sEn sección crítica\n", processTag);
    usleep(SC_WAIT * 1000);
    printf("%sSaliendo de sección crítica\n", processTag);
    sndTicketToLauncher(TYPE_EXIT_CS, ticket);
}

void initNode(int argc, char *argv[]) {
    if (argc != 4) {
        printWrongUsageError();
        exit(0);
    }
    nodeID = atoi(argv[1]);
    totalNodes = atoi(argv[2]);
    priority = atoi(argv[3]);
    if (nodeID > totalNodes) {
        printWrongUsageError();
    }
    pid = getpid();
    sprintf(processTag, "N%d %d> ", nodeID, pid);

    sharedMemoryPointer = getSharedMemory(nodeID);
}

void printWrongUsageError() {
    printf("Wrong arguments\nUsage: ./Process priority nodeID totalNodes mode (nodeID <= totalNodes)\n");
}

#pragma clang diagnostic pop
