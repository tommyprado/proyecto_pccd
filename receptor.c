#include <stdio.h>
#include <stdlib.h>
#include "headers/coms.h"
#include "headers/inits.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

sharedMemStruct sharedMemory;

char receptorTag[100];
int nodeID, totalNodes;

void updateMaxPetitionID (int petitionId);

void saveRequest (ticket ticket);

void initReceptor(int argc, char *argv[]);

void printWrongUsageError();

int main(int argc, char *argv[]){
    initReceptor(argc, argv);
    while(1) {
        printf("%sEsperando a recibir mensaje\n", receptorTag);
        ticket originTicket = receiveRequest(nodeID);
        printf("%sMensaje recibido\n", receptorTag);
        updateMaxPetitionID(sharedMemory.maxPetition);
        sem_wait(&sharedMemory.semWantTo);
        sem_wait(&sharedMemory.semTicket);
        if(!sharedMemory.wantTo || sharedMemory.wantTo && (compTickets(sharedMemory.myTicket, originTicket) == 1) ) { // myTicket > originTicket?
            sem_post(&sharedMemory.semTicket);
            sem_post(&sharedMemory.semWantTo);
            printf("%sEnviando reply\n", receptorTag);
            sendReply(originTicket);
        } else{
            sem_post(&sharedMemory.semTicket);
            printf("%sGuardando request\n", receptorTag);
            saveRequest(originTicket);
            sem_post(&sharedMemory.semWantTo);
        }
    }
}

void initReceptor(int argc, char *argv[]) {
    if (argc != 2) {
        printWrongUsageError();
        exit(0);
    }
    nodeID = atoi(argv[1]);
    totalNodes = atoi(argv[2]);
    if (nodeID > totalNodes) {
        printWrongUsageError();
    }
    sprintf(receptorTag, "RECEPTOR %d> ", nodeID);
    initMailBoxes(nodeID);
    sharedMemory = *initSharedMemory(nodeID);
}

void printWrongUsageError() {
    printf("Wrong arguments\nUsage: ./main nodeID totalNodes mode (nodeID <= totalNodes)\n");
}

void updateMaxPetitionID (int petitionId){
    sem_wait(&sharedMemory.semMaxPetition);
    if(petitionId >= sharedMemory.maxPetition){
        sharedMemory.maxPetition = petitionId + 1;
    }
    sem_post(&sharedMemory.semMaxPetition);
}

void saveRequest (ticket ticket){
    sem_wait(&sharedMemory.semPending);
    sharedMemory.pendingRequestsArray[sharedMemory.pendingRequestsCount] = ticket;
    sharedMemory.pendingRequestsCount++;
    sem_post(&sharedMemory.semPending);
}

#pragma clang diagnostic pop