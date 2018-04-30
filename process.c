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

void waitForCSAccess();

ticket createTicket();

void wakeUpNextProcess(int priority);

bool nodeHasProcesses();

void addProcessToCount(int priority);

void removeProcessFromCount(int priority);

void replyPending(int priority, int nodeID);

bool priorityHasProcesses(int priority);

void addPriorityTicket(ticket ticket);

int startCompetition(bool hasToSend) ;

sharedMemory *sharedMemoryPointer;

int totalNodes, nodeID, pid, priority;

char processTag[100];

int main(int argc, char *argv[]){
    initNode(argc, argv);
    sem_wait(&sharedMemoryPointer->nodeStatusSem);
    int competitionStatus = 0;
    if (nodeHasProcesses()) {
        printf("%sPrimer proceso\n", processTag);
        addProcessToCount(priority);
        competitionStatus = startCompetition(true);

    } else if (priority < sharedMemoryPointer->competitorTicket.priority) {
        printf("Proceso más prioritario que el anterior\n");
        addProcessToCount(priority);
        competitionStatus = startCompetition(false);
    } else if (!priorityHasProcesses(priority)){
        printf("Primer proceso (no prioritario)\n");
        ticket ticket = createTicket();
        addPriorityTicket(ticket);
    } else {
        addProcessToCount(priority);
        sem_post(&sharedMemoryPointer->nodeStatusSem);
        waitForCSAccess();
    }

    if (competitionStatus == PROCESS_PASSED){
        waitForCSAccess();
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

void replyPending(int priority, int nodeID){
    if (priority <= PAGOS) {
        for (int i = 0; i < sharedMemoryPointer->pendingPagosCount; ++i) {
            sendReply(sharedMemoryPointer->pendingPagosArray[i], nodeID);
        }
        sharedMemoryPointer->pendingPagosCount = 0;
    }
    if (priority <= ANULACIONES) {
        for (int i = 0; i < sharedMemoryPointer->pendingAnulacionesCount; ++i) {
            sendReply(sharedMemoryPointer->pendingAnulacionesArray[i], nodeID);
        }
        sharedMemoryPointer->pendingAnulacionesCount = 0;
    }
    if (priority <= RESERVAS) {
        for (int i = 0; i < sharedMemoryPointer->pendingReservasCount; ++i) {
            sendReply(sharedMemoryPointer->pendingReservasArray[i], nodeID);
        }
        sharedMemoryPointer->pendingReservasCount = 0;
    }
    if (priority <= CONSULTORES) {
        for (int i = 0; i < sharedMemoryPointer->pendingConsultoresCount; ++i) {
            sendReply(sharedMemoryPointer->pendingConsultoresArray[i], nodeID);
        }
        sharedMemoryPointer->pendingConsultoresCount = 0;
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

void addProcessToCount(int priority) {
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

int startCompetition(bool hasToSend) {
    ticket ticket = createTicket();
    addPriorityTicket(ticket);
    if (hasToSend) {
        sharedMemoryPointer->competitorTicket = ticket;
        sendRequests(sharedMemoryPointer->competitorTicket, totalNodes);
    }
    sem_post(&sharedMemoryPointer->nodeStatusSem);
    int countReply = 0;
    int status = 0;
    while (countReply < totalNodes - 1) {
        long node = receiveReply(nodeID, pid);
        sem_wait(&sharedMemoryPointer->nodeStatusSem);
        if (sharedMemoryPointer->competitorTicket.priority == priority) {
            countReply++;
            printf("%sRecibido reply from %ld\n", processTag, node);
        } else {
            printf("%sReply inválido\n", processTag);
            status = PROCESS_PASSED;
            break;
        }
        sem_post(&sharedMemoryPointer->nodeStatusSem);
    }
    return status;
}

void addPriorityTicket(ticket ticket) {
    switch (priority) {
        case PAGOS:
            sharedMemoryPointer->pagosTicket = ticket;
            break;
        case ANULACIONES:
            sharedMemoryPointer->anulacionesTicket = ticket;
            break;
        case RESERVAS:
            sharedMemoryPointer->reservasTicket = ticket;
            break;
        case CONSULTORES:
            sharedMemoryPointer->consultoresTicket = ticket;
            break;
        default:
            printf("Prioridad incorrecta\n");
            exit(1);
    }
}

ticket createTicket() {
    sem_wait(&sharedMemoryPointer->competitorTicketSem);
    sharedMemoryPointer->maxRequestID = sharedMemoryPointer->maxRequestID + 1;
    ticket ticket = {.nodeID = nodeID, .requestID = sharedMemoryPointer->maxRequestID, .pid=pid, .priority = priority};
    sem_post(&sharedMemoryPointer->competitorTicketSem);
    return ticket;
}

void waitForCSAccess() {
    switch (priority) {
        case PAGOS:
            sharedMemoryPointer->nextPagosCount = sharedMemoryPointer->nextPagosCount + 1;
            sem_wait(&sharedMemoryPointer->nextPagosSem);
            break;
        case ANULACIONES:
            sharedMemoryPointer->nextAnulacionesCount = sharedMemoryPointer->nextAnulacionesCount + 1;
            sem_wait(&sharedMemoryPointer->nextAnulacionesSem);
            break;
        case RESERVAS:
            sharedMemoryPointer->nextReservasCount = sharedMemoryPointer->nextReservasCount + 1;
            sem_wait(&sharedMemoryPointer->nextReservasSem);
            break;
        case CONSULTORES:
            sharedMemoryPointer->nextConsultoresCount = sharedMemoryPointer->nextConsultoresCount + 1;
            sem_wait(&sharedMemoryPointer->nextConsultoresSem);
            break;
        default:
            printf("Prioridad incorrecta\n");
            exit(1);
    }

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
