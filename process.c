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

#define PROCESS_PASSED -1

void initNode(int argc, char *argv[]);

void printWrongUsageError();

void updateRequestID(int originRequestID);

void saveRequest (ticket ticket);

void accessCS (ticket ticket);

ticket createTicket();

void wakeUpNextProcess(int priority);

bool nodeHasProcesses();

void addProcessToCount();

void removeProcessFromCount(int priority);

void replyPending(int priority, int nodeID);

bool priorityHasProcesses(int priority);

void waitByPriority();

void postByPriority();

sharedMemory *sharedMemoryPointer;

int totalNodes, nodeID, pid, priority;

char processTag[100];

int main(int argc, char *argv[]){
    initNode(argc, argv);

    while (1) {
        sem_wait(&sharedMemoryPointer->nodeStatusSem);
        if (!nodeHasProcesses()) {
            addProcessToCount();
            postByPriority();
            sem_post(&sharedMemoryPointer->nodeStatusSem);
            break;
        }
        sem_post(&sharedMemoryPointer->nodeStatusSem);

        waitByPriority();

        sem_wait(&sharedMemoryPointer->nodeStatusSem);
        ticket ticket = createTicket();
        if (compTickets(ticket, sharedMemoryPointer->competitorTicket) == -1) {

        }
        sem_post(&sharedMemoryPointer->nodeStatusSem);

    }

    accessCS(sharedMemoryPointer->competitorTicket);

    sem_wait(&sharedMemoryPointer->nodeStatusSem);
    removeProcessFromCount(priority);
    if (sharedMemoryPointer->pendingRequestsMaxPriority >= priority) {
        wakeUpNextProcess(priority);
    } else {
        replyPending(priority, nodeID);
    }
    sem_post(&sharedMemoryPointer->nodeStatusSem);

    sndMsgToLauncher(TYPE_PROCESS_FINISHED);
}

void postByPriority() {
    switch (priority) {
        case PAGOS:
            sem_post(&sharedMemoryPointer->nextPagosSem);
        case ANULACIONES:
            sem_post(&sharedMemoryPointer->nextAnulacionesSem);
        case RESERVAS:
            sem_post(&sharedMemoryPointer->nextReservasSem);
        case CONSULTORES:
            sem_post(&sharedMemoryPointer->nextConsultoresSem);
        default:
            printf("Error en las prioridades\n");
    }
}

void waitByPriority() {
    switch (priority) {
        case PAGOS:
            sem_wait(&sharedMemoryPointer->nextPagosSem);
        case ANULACIONES:
            sem_wait(&sharedMemoryPointer->nextAnulacionesSem);
        case RESERVAS:
            sem_wait(&sharedMemoryPointer->nextReservasSem);
        case CONSULTORES:
            sem_wait(&sharedMemoryPointer->nextConsultoresSem);
        default:
            printf("Error en las prioridades\n");
    }
}

bool priorityHasProcesses(int priority) {
    switch (priority) {
        case PAGOS:
            return sharedMemoryPointer->nextPagosCount == 0;
        case ANULACIONES:
            return sharedMemoryPointer->nextAnulacionesCount == 0;
        case RESERVAS:
            return sharedMemoryPointer->nextReservasCount == 0;
        case CONSULTORES:
            return sharedMemoryPointer->nextConsultoresCount == 0;
        default:
            printf("Error en las prioridades\n");
    }
}

void removeProcessFromCount(int priority) {
    switch (priority) {
        case PAGOS:
            sharedMemoryPointer->nextPagosCount = sharedMemoryPointer->nextPagosCount - 1;
            break;
        case ANULACIONES:
            sharedMemoryPointer->nextAnulacionesCount = sharedMemoryPointer->nextAnulacionesCount - 1;
            break;
        case RESERVAS:
            sharedMemoryPointer->nextReservasCount = sharedMemoryPointer->nextReservasCount - 1;
            break;
        case CONSULTORES:
            sharedMemoryPointer->nextConsultoresCount = sharedMemoryPointer->nextConsultoresCount - 1;
            break;
        default:
            printf("Error en las prioridades\n");
    }
}

void addProcessToCount() {
    switch (priority) {
        case PAGOS:
            sharedMemoryPointer->nextPagosCount = sharedMemoryPointer->nextPagosCount + 1;
            break;
        case ANULACIONES:
            sharedMemoryPointer->nextAnulacionesCount = sharedMemoryPointer->nextAnulacionesCount + 1;
            break;
        case RESERVAS:
            sharedMemoryPointer->nextReservasCount = sharedMemoryPointer->nextReservasCount + 1;
            break;
        case CONSULTORES:
            sharedMemoryPointer->nextConsultoresCount = sharedMemoryPointer->nextConsultoresCount + 1;
            break;
        default:
            printf("Error en las prioridades\n");
    }
}

bool nodeHasProcesses() {
    return
            sharedMemoryPointer->nextPagosCount == 0 &&
            sharedMemoryPointer->nextAnulacionesCount == 0 &&
            sharedMemoryPointer->nextReservasCount == 0 &&
            sharedMemoryPointer->nextConsultoresCount == 0;
}

void wakeUpNextProcess(int priority) {
    if (priority <= PAGOS && sharedMemoryPointer->nextPagosCount > 0) {
        sem_post(&sharedMemoryPointer->nextPagosSem);
    }
    else if (priority <= ANULACIONES && sharedMemoryPointer->nextAnulacionesCount > 0) {
        sem_post(&sharedMemoryPointer->nextAnulacionesSem);
    }
    else if (priority <= RESERVAS && sharedMemoryPointer->nextReservasCount > 0) {
        sem_post(&sharedMemoryPointer->nextReservasSem);
    }
    else if (priority <= CONSULTORES && sharedMemoryPointer->nextConsultoresCount > 0) {
        sem_post(&sharedMemoryPointer->nextConsultoresSem);
    }
}

ticket createTicket() {
    sem_wait(&sharedMemoryPointer->competitorTicketSem);
    sharedMemoryPointer->maxRequestID = sharedMemoryPointer->maxRequestID + 1;
    ticket ticket = {.nodeID = nodeID, .requestID = sharedMemoryPointer->maxRequestID, .pid=pid, .priority = priority};
    sem_post(&sharedMemoryPointer->competitorTicketSem);
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
