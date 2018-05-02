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

#define SC_WAIT 10

void initNode(int argc, char *argv[]);

void printWrongUsageError();

void updateRequestID(int originRequestID);

void saveRequest (ticket ticket);

void accessCS (ticket ticket);

ticket createTicket();

void wakeNextInLine();

void replyPendingRequests(ticket mTicket) ;

void replyAll();

void resetCompetitor(sharedMemory *sharedMemoryPointer);

void replyPendingRequestsConsult();

sharedMemory *sharedMemoryPointer;

int totalNodes, nodeID, pid, priority;

char processTag[100];

char mTicketString[100];

bool reset = false;

int main(int argc, char *argv[]){
    initNode(argc, argv);
    char ticketString1[100];
    char ticketString2[100];
    ticket mTicket;
    while (1) {
        sem_wait(&sharedMemoryPointer->nodeStatusSem);
        reset = false;
        if (!nodeHasProcesses(sharedMemoryPointer)) {
            addProcessToCount(sharedMemoryPointer, priority);
        }
        else if (priority == CONSULTORES && sharedMemoryPointer->consultorsInSC > 0) {
            addProcessToCount(sharedMemoryPointer, priority);
        }
        else if (&sharedMemoryPointer->inSC || priority >= sharedMemoryPointer->competitorTicket.priority) {
            addProcessToCount(sharedMemoryPointer, priority);
            sem_post(&sharedMemoryPointer->nodeStatusSem);
            waitByPriority(sharedMemoryPointer, priority);
            sem_wait(&sharedMemoryPointer->nodeStatusSem);
        }
        mTicket = createTicket();
        ticketToString(mTicketString, mTicket);
        if (priority == CONSULTORES && sharedMemoryPointer->consultorsInSC > 0) {
            break;
        }
        sharedMemoryPointer->competitorTicket = mTicket;

        ticketToString(ticketString1, mTicket);
        printf("%sPidiendo acceso para %s\n", processTag, ticketString1);
        replyPendingRequests(mTicket);
        sendRequests(mTicket, totalNodes);
        printf("%sEsperando replies...\n", processTag);
        int replyCont = 0;
        while (replyCont < totalNodes - 1) {
            sem_post(&sharedMemoryPointer->nodeStatusSem);
            ticketMessage message;
            receiveReply(mTicket, &message);
            sem_wait(&sharedMemoryPointer->nodeStatusSem);
            if (compTickets(mTicket, message.ticket) != 0) {
                ticketToString(ticketString2, message.ticket);
                continue;
            }
            printf("%sRecibido el reply número %d del nodo %i para %s\n", processTag, replyCont + 1, message.origin, ticketString1);
            if ((priority == CONSULTORES && sharedMemoryPointer->competitorTicket.priority != CONSULTORES) ||
                compTickets(sharedMemoryPointer->competitorTicket, mTicket) != 0) {
                char aux[200];
                ticketToString(aux, sharedMemoryPointer->competitorTicket);
                printf("%sEl ticket %s ya no es válido\n",processTag, ticketString1);
                wakeNextInLine();
                removeProcessFromCount(sharedMemoryPointer, priority);
                reset = true;
                break;
            }
            replyCont++;
        }
        if (reset) {
            sem_post(&sharedMemoryPointer->nodeStatusSem);
            continue;
        }
        if(priority == CONSULTORES) {
            printf("%sPrimer consultor ha entrado %s\n", processTag, mTicketString);
            replyPendingRequestsConsult();
        }
        sharedMemoryPointer->inSC = true;
        break;
    }
    if(priority == CONSULTORES){
        sharedMemoryPointer->consultorsInSC = sharedMemoryPointer->consultorsInSC + 1;
        sharedMemoryPointer->pendingConsultors = sharedMemoryPointer->pendingConsultors - 1;
        if(sharedMemoryPointer->pendingConsultors > 0){
            sem_post(&sharedMemoryPointer->nextConsultoresSem);
        }
    }
    sem_post(&sharedMemoryPointer->nodeStatusSem);
    accessCS(mTicket);
    sem_wait(&sharedMemoryPointer->nodeStatusSem);
    if (priority == CONSULTORES && sharedMemoryPointer->consultorsInSC > 1) {
        sharedMemoryPointer->consultorsInSC = sharedMemoryPointer->consultorsInSC - 1;
        removeProcessFromCount(sharedMemoryPointer, priority);
    } else if (priority != CONSULTORES || (priority == CONSULTORES && sharedMemoryPointer->consultorsInSC == 1)) {
        if (priority == CONSULTORES) {
            sharedMemoryPointer->consultorsInSC = sharedMemoryPointer->consultorsInSC - 1;
        }
        sharedMemoryPointer->inSC = false;
        removeProcessFromCount(sharedMemoryPointer, priority);
        resetCompetitor(sharedMemoryPointer);
        if (nodeHasProcesses(sharedMemoryPointer)){
            wakeNextInLine();
        } else {
            replyAll();
        }
    }
    sem_post(&sharedMemoryPointer->nodeStatusSem);
}

void resetCompetitor(sharedMemory *sharedMemoryPointer) {
    ticket ticket;
    ticket.priority = NONE;
    sharedMemoryPointer->competitorTicket = ticket;
}

void replyAll() {
    ticket ticket;
    ticket.priority = NONE;
    replyPendingRequests(ticket);
}

void replyPendingRequests(ticket mTicket) {
    ticket newPending[PENDING_REQUESTS_LIMIT];
    char aux [100];
    int newPendingCount = 0;
    for (int i = 0; i < sharedMemoryPointer->pendingRequestsCount; ++i) {
        ticket pendingRequest = sharedMemoryPointer->pendingRequests[i];
        ticketToString(aux, pendingRequest);
        if (compTickets(pendingRequest, mTicket) == -1) {
            printf("%sEnviando reply %s\n", processTag, aux);
            sendReply(pendingRequest, nodeID);
        } else {
            printf("%sGuardando en nuevo array %s\n", processTag, aux);
            newPending[newPendingCount] = pendingRequest;
            newPendingCount++;
        }
    }

    for (int j = 0; j < newPendingCount; ++j) {
        sharedMemoryPointer->pendingRequests[j] = newPending[j];
    }
    sharedMemoryPointer->pendingRequestsCount = newPendingCount;
}

void replyPendingRequestsConsult() {
    ticket newPending[PENDING_REQUESTS_LIMIT];
    char aux [100];
    int newPendingCount = 0;
    for (int i = 0; i < sharedMemoryPointer->pendingRequestsCount; ++i) {
        ticket pendingRequest = sharedMemoryPointer->pendingRequests[i];
        ticketToString(aux, pendingRequest);
        if (pendingRequest.priority == CONSULTORES) {
            printf("%sEnviando reply del nodo %i para %s\n", processTag, nodeID, aux);
            sendReply(pendingRequest, nodeID);
        } else {
            printf("%sGuardando en nuevo array %s\n", processTag, aux);
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
    ticketToString(mTicketString, ticket);
    printf("%sEn sección crítica con ticket %s\n", processTag, mTicketString);
    usleep(SC_WAIT * 1000);
    printf("%sSaliendo de sección crítica con ticket %s\n", processTag, mTicketString);
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
