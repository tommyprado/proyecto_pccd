//
// Created by mati on 30/04/18.
//

#ifndef PROYECTO_PRIORITYUTILS_H
#define PROYECTO_PRIORITYUTILS_H

#include "inits.h"

bool priorityHasProcesses(sharedMemory *sharedMemoryPointer, int priority);

void removeProcessFromCount(sharedMemory *sharedMemoryPointer, int priority);

void addProcessToCount(sharedMemory *sharedMemoryPointer, int priority);

bool nodeHasProcesses(sharedMemory *sharedMemoryPointer);

void waitByPriority(sharedMemory *sharedMemoryPointer, int priority);

void postByPriority(sharedMemory *sharedMemoryPointer, int priority);

#endif //PROYECTO_PRIORITYUTILS_H
