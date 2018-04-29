#include <stdio.h>
#include <stdlib.h>
#include "headers/coms.h"
#include "headers/inits.h"
#include "headers/launcherUtils.h"
#include "headers/ticketUtils.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

sharedMemStruct *sharedMemoryPointer;

char receptorTag[100];
int nodeID;

void updateMaxPetition(int petitionId);

void saveRequest (ticket ticket);

void initReceptor(int argc, char *argv[]);

void printWrongUsageError();

int main(int argc, char *argv[]){
    initReceptor(argc, argv);
    while(1) {
        printf("%sEsperando a recibir mensaje\n", receptorTag);
        ticket originTicket = receiveRequest(nodeID);
        printf("%sMensaje recibido de %d\n", receptorTag, originTicket.pid);
        updateMaxPetition(sharedMemoryPointer->maxPetition);
        sem_wait(&sharedMemoryPointer->semWantTo);
        sem_wait(&sharedMemoryPointer->semTicket);
        if(!sharedMemoryPointer->wantTo ||
           (sharedMemoryPointer->wantTo && (compTickets(sharedMemoryPointer->myTicket, originTicket) == 1))) { // myTicket > originTicket?
            sem_post(&sharedMemoryPointer->semTicket);
            sem_post(&sharedMemoryPointer->semWantTo);
            printf("%sEnviando reply a %d\n", receptorTag, originTicket.pid);
            sendReply(originTicket);
        } else{
            sem_post(&sharedMemoryPointer->semTicket);
            printf("%sGuardando request\n", receptorTag);
            saveRequest(originTicket);
            sem_post(&sharedMemoryPointer->semWantTo);
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

void updateMaxPetition(int petitionId){
    sem_wait(&sharedMemoryPointer->semMaxPetition);
    if(petitionId >= sharedMemoryPointer->maxPetition){
        sharedMemoryPointer->maxPetition = petitionId + 1;
    }
    sem_post(&sharedMemoryPointer->semMaxPetition);
}

void saveRequest (ticket ticket){
    sem_wait(&sharedMemoryPointer->semPending);
    sharedMemoryPointer->pendingRequestsArray[sharedMemoryPointer->pendingRequestsCount] = ticket;
    sharedMemoryPointer->pendingRequestsCount++;
    sem_post(&sharedMemoryPointer->semPending);
}

#pragma clang diagnostic pop