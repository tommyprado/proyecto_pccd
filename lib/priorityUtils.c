#include <stdio.h>
#include "../headers/priorityUtils.h"
#include "../headers/inits.h"

bool priorityHasProcesses(sharedMemory *sharedMemoryPointer, int priority) {
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
            printf("Prioridad %d no soportada (priorityHasProcesses)\n", priority);
    }
}

void removeProcessFromCount(sharedMemory *sharedMemoryPointer, int priority) {
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
            printf("Prioridad %d no soportada(removeProcessToCount)\n", priority);
    }
}

void addProcessToCount(sharedMemory *sharedMemoryPointer, int priority) {
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
            printf("Prioridad %d no soportada (addProcessToCount)\n", priority);
    }
}

bool nodeHasProcesses(sharedMemory *sharedMemoryPointer) {
    return
            sharedMemoryPointer->nextPagosCount != 0 ||
            sharedMemoryPointer->nextAnulacionesCount != 0 ||
            sharedMemoryPointer->nextReservasCount != 0 ||
            sharedMemoryPointer->nextConsultoresCount != 0;
}

void waitByPriority(sharedMemory *sharedMemoryPointer, int priority) {
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
            printf("Prioridad %d no soportada (waitByPriority)\n", priority);
    }
}

void postByPriority(sharedMemory *sharedMemoryPointer, int priority) {
    switch (priority) {
        case PAGOS:
            sem_post(&sharedMemoryPointer->nextPagosSem);
            break;
        case ANULACIONES:
            sem_post(&sharedMemoryPointer->nextAnulacionesSem);
            break;
        case RESERVAS:
            sem_post(&sharedMemoryPointer->nextReservasSem);
            break;
        case CONSULTORES:
            sem_post(&sharedMemoryPointer->nextConsultoresSem);
            break;
        default:
            printf("Prioridad %d no soportada (postByPriority)\n", priority);
    }
}