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

#define SC_WAIT 50

void initNode(int argc, char *argv[]);

void printWrongUsageError();

void updateRequestID(int originRequestID);

void saveRequest (ticket ticket);

void accessCS (ticket ticket);

ticket createTicket();

void wakeNextInLine();

void replyPendingRequests(ticket mTicket) ;

void replyAll();

void resetCompetitor(sharedMemory *pointer);

void replyPendingRequestsConsult();

sharedMemory *sharedMemoryPointer;

int totalNodes, nodeID, pid, priority;

char processTag[100];

char stringTicket[10];

bool reset = false;

int main(int argc, char *argv[]){
    initNode(argc, argv);
    char ticketString1[100];
    char ticketString2[100];
    while (1) {
        sem_wait(&sharedMemoryPointer->nodeStatusSem);
        if (!nodeHasProcesses(sharedMemoryPointer)) {
            addProcessToCount(sharedMemoryPointer, priority);
            printf("%sNodo vacío, entrando con prioridad %d\n", processTag, priority);
        }
        else if (&sharedMemoryPointer->inSC || priority >= sharedMemoryPointer->competitorTicket.priority) {
            printf("%sPrioridad %d ya en juego, esperando...\n", processTag, priority);
            addProcessToCount(sharedMemoryPointer, priority);
            sem_post(&sharedMemoryPointer->nodeStatusSem);
            waitByPriority(sharedMemoryPointer, priority);
            sem_wait(&sharedMemoryPointer->nodeStatusSem);
        }
        ticket mTicket = createTicket();
        if (compTickets(mTicket, sharedMemoryPointer->competitorTicket) == -1) {
            if (sharedMemoryPointer->competitorTicket.priority != NONE) {
                ticketToString(ticketString1, sharedMemoryPointer->competitorTicket);
                ticketToString(ticketString2, mTicket);
                printf("%sSustituyendo %s por %s\n", processTag, ticketString1, ticketString2);
            }
            sharedMemoryPointer->competitorTicket = mTicket;
        }

        ticketToString(ticketString1, mTicket);
        printf("%sPidiendo acceso para %s\n", processTag, ticketString1);
        replyPendingRequests(mTicket);
        sendRequests(mTicket, totalNodes);
        printf("%sEsperando replies...\n", processTag);
        for (int i = 1; i < totalNodes; ++i) {
            sem_post(&sharedMemoryPointer->nodeStatusSem);
            receiveReply(mTicket);
            sem_wait(&sharedMemoryPointer->nodeStatusSem);
            printf("%sRecibido el reply número %d\n", processTag, i);
            if (compTickets(sharedMemoryPointer->competitorTicket, mTicket) != 0) {
                char aux[200], aux2[2];
                ticketToString(aux, sharedMemoryPointer->competitorTicket);
                ticketToString(aux2, mTicket);
                printf("%sReset debido a %s en %s\n",processTag, aux, aux2);
                if (!sharedMemoryPointer->inSC) {
                    wakeNextInLine();
                }
                removeProcessFromCount(sharedMemoryPointer, priority);
                reset = true;
                break;
            }
        }
        if (reset) {
            reset = false;
            sem_post(&sharedMemoryPointer->nodeStatusSem);
            continue;
        }
        if(sharedMemoryPointer->competitorTicket.priority == CONSULTORES) {
            replyPendingRequestsConsult();
        }
        sharedMemoryPointer->inSC = true;
        sem_post(&sharedMemoryPointer->nodeStatusSem);
        break;
    }

    accessCS(sharedMemoryPointer->competitorTicket);
    sem_wait(&sharedMemoryPointer->nodeStatusSem);
    sharedMemoryPointer->inSC = false;
    removeProcessFromCount(sharedMemoryPointer, priority);
    resetCompetitor(sharedMemoryPointer);
    if (nodeHasProcesses(sharedMemoryPointer)){
        wakeNextInLine();
    } else {
        replyAll();
    }
    sem_post(&sharedMemoryPointer->nodeStatusSem);
    sndMsgToLauncher(TYPE_PROCESS_FINISHED);
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
    int newPendingCount = 0;
    for (int i = 0; i < sharedMemoryPointer->pendingRequestsCount; ++i) {
        ticket pendingRequest = sharedMemoryPointer->pendingRequests[i];
        if (compTickets(pendingRequest, mTicket) == -1) {
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

void replyPendingRequestsConsult() {
    ticket newPending[PENDING_REQUESTS_LIMIT];
    char aux [100];
    int newPendingCount = 0;
    for (int i = 0; i < sharedMemoryPointer->pendingRequestsCount; ++i) {
        ticket pendingRequest = sharedMemoryPointer->pendingRequests[i];
        ticketToString(aux, pendingRequest);
        if (pendingRequest.priority == CONSULTORES) {
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
