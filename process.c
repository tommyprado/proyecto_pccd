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

#define SC_WAIT 500

void initNode(int argc, char *argv[]);

void printWrongUsageError();

void updateRequestID(int originRequestID);

void saveRequest (ticket ticket);

void accessCS (ticket ticket);

ticket createTicket();

void wakeNextInLine();

void replyPendingRequests(ticket mTicket) ;

void replyAll();

sharedMemory *sharedMemoryPointer;

int totalNodes, nodeID, pid, priority;

char processTag[100];

char stringTicket[10];

int main(int argc, char *argv[]){
    initNode(argc, argv);

    while (1) {
        sem_wait(&sharedMemoryPointer->nodeStatusSem);
        if (!nodeHasProcesses(sharedMemoryPointer)) {
            printf("%sNodo vacío, entrando con prioridad %d\n", processTag, priority);
            addProcessToCount(sharedMemoryPointer, priority);
            sem_post(&sharedMemoryPointer->nodeStatusSem);
        }
        else if (priority >= sharedMemoryPointer->competitorTicket.priority) {
            printf("%sProceso más prioritario que el actual", processTag);
            addProcessToCount(sharedMemoryPointer, priority);
            waitByPriority(sharedMemoryPointer, priority);
            sem_post(&sharedMemoryPointer->nodeStatusSem);
        }
        sem_wait(&sharedMemoryPointer->nodeStatusSem);
        ticket mTicket = createTicket();
        if (compTickets(mTicket, sharedMemoryPointer->competitorTicket) == 1) {
            sharedMemoryPointer->competitorTicket = mTicket;
        }
        char ticketString[100];
        ticketToString(ticketString, mTicket);
        printf("%sPidiendo acceso para %s\n", processTag, ticketString);
        replyPendingRequests(mTicket);
        sendRequests(mTicket, totalNodes);
        printf("%sEsperando replies...\n", processTag);
        for (int i = 1; i < totalNodes; ++i) {
            sem_post(&sharedMemoryPointer->nodeStatusSem);
            receiveReply(mTicket);
            sem_wait(&sharedMemoryPointer->nodeStatusSem);
            printf("%sRecibido %d reply\n", processTag, i + 1);
            if (compTickets(sharedMemoryPointer->competitorTicket, mTicket) != 0) {
                char aux[200], aux2[2];
                ticketToString(aux, sharedMemoryPointer->competitorTicket);
                ticketToString(aux2, mTicket);
                printf("%s %s\n", aux, aux2);
                wakeNextInLine();
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
    if (nodeHasProcesses(sharedMemoryPointer)){
        wakeNextInLine();
    } else {
        replyAll();
    }
    sem_post(&sharedMemoryPointer->nodeStatusSem);

    sndMsgToLauncher(TYPE_PROCESS_FINISHED);
}

void replyAll() {
    ticket ticket;
    ticket.priority = NONE;
    replyPendingRequests(ticket);
}

void replyPendingRequests(ticket mTicket) {
    ticket newPending[PENDING_REQUESTS_LIMIT];
    int newPendingCount = 0;
    for (int i = 0; i < sharedMemoryPointer->pendingRequestsCount; ++i) {
        ticket pendingRequest = sharedMemoryPointer->pendingRequests[i];
        if (compTickets(pendingRequest, mTicket) == 1) {
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
    ticketToString(stringTicket, sharedMemoryPointer->competitorTicket);
    printf("%sEn sección crítica con ticket %s\n", processTag, stringTicket);
    usleep(SC_WAIT * 1000);
    printf("%sSaliendo de sección crítica con ticket %s\n", processTag, stringTicket);
    sndTicketToLauncher(TYPE_EXIT_CS, ticket);
}

void initNode(int argc, char *argv[]) {
    if (argc != 4) {
        printWrongUsageError();
        exit(0);
    }
    priority = atoi(argv[1]);
    nodeID = atoi(argv[2]);
    totalNodes = atoi(argv[3]);
    if (nodeID > totalNodes) {
        printWrongUsageError();
    }
    pid = getpid();
    sprintf(processTag, "N%d -> ", nodeID);

    sharedMemoryPointer = getSharedMemory(nodeID);
}

void printWrongUsageError() {
    printf("Wrong arguments\nUsage: ./Process priority nodeID totalNodes (nodeID <= totalNodes)\n");
}

#pragma clang diagnostic pop
