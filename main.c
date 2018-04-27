#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <zconf.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <pthread.h>
#include <sys/time.h>
#include "headers/ticketUtils.h"
#include "headers/inits.h"
#include "headers/coms.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

#define PENDING_REQUESTS_LIMIT 1000000
#define NODE_INITIAL_KEY 10000
#define COMMON_MAILBOX_KEY 283300
#define TYPE_ENTRO 3
#define TYPE_SALGO 4

#define MANUAL 0
#define SC_WAIT 100



void setWantTo (int value);

void initNode(int argc, char *argv[]);

void printWrongUsageError();

void updateMaxPetitionID (int petitionId);

void saveRequest (ticket ticket);

void doStuff(int mode) ;

void accessCS (int mode,ticket ticket);

int totalNodes, maxPetition, wantTo, nodeID;
sem_t semMaxPetition, semWantTo, semPending, semTicket;
ticket pendingRequestsArray[PENDING_REQUESTS_LIMIT];
int pendingRequestsCount;
ticket myTicket;

int mode;
char mainTag[100];
char receptorTag[100];

int main(int argc, char *argv[]){
    initNode(argc, argv);
    while (1) {
        printf("%sMoviendo papeles...\n", mainTag);
        doStuff(mode);
        printf("%sIntentando acceder a la sección crítica...\n", mainTag);
        setWantTo(1);
        sem_wait(&semTicket);
        myTicket = createTicket(nodeID, &maxPetition, &semMaxPetition);
        sendRequests(myTicket, nodeID, totalNodes);
        sem_post(&semTicket);
        int countReply = 1;
        while (countReply < totalNodes) {
            receiveReply(nodeID);
            printf("%sReply recibido\n", mainTag);
            countReply++;
        }
        printf("%sAccediendo a la sección crítica...\n", mainTag);
        accessCS(0, myTicket);
        printf("%sFuera de la sección crítica\n", mainTag);
        setWantTo(0);
        printf("%sRespondiendo a Requests pendientes...\n", mainTag);
        replyAllPending(&semPending, &pendingRequestsCount, pendingRequestsArray, nodeID);
    }
}

void * receptorMain(void *arg) {
    while(1) {
        printf("%sEsperando a recibir mensaje\n", receptorTag);
        ticket originTicket = receiveRequest(nodeID);
        printf("%sMensaje recibido\n", receptorTag);
        updateMaxPetitionID(maxPetition);
        sem_wait(&semWantTo);
        sem_wait(&semTicket);
        if(!wantTo || wantTo && (compTickets(myTicket, originTicket) == 1) ) { // myTicket > originTicket?
            sem_post(&semTicket);
            sem_post(&semWantTo);
            printf("%sEnviando reply\n", receptorTag);
            sendReply(originTicket);
        } else{
            sem_post(&semTicket);
            printf("%sGuardando request\n", receptorTag);
            saveRequest(originTicket);
            sem_post(&semWantTo);
        }
    }
}

void updateMaxPetitionID (int petitionId){
    sem_wait(&semMaxPetition);
    if(petitionId >= maxPetition){
        maxPetition = petitionId + 1;
    }
    sem_post(&semMaxPetition);
}

void saveRequest (ticket ticket){
    sem_wait(&semPending);
    pendingRequestsArray[pendingRequestsCount] = ticket;
    pendingRequestsCount++;
    sem_post(&semPending);
}

void setWantTo (int value){
    sem_wait(&semWantTo);
    wantTo=value;
    sem_post(&semWantTo);
}

void doStuff(int mode) {
    if(mode == MANUAL){
        getchar();
        return;
    }
}

void writeOut(int tipo, ticket ticket) {

    struct timeval tv;
    gettimeofday(&tv,NULL);
    __suseconds_t t = tv.tv_usec;
    printMessage message;
    message.mtype = tipo;
    message.ticket = ticket;
    message.t = t;

    if(msgsnd(COMMON_MAILBOX_KEY, &message, sizeof(ticket)+sizeof(t), 0) == -1) {
        printf("Error al invocar 'msgrcv()'.\n");
        exit(0);
    }

}


void accessCS (int mode, ticket ticket){
    writeOut(TYPE_ENTRO, ticket);
    if(mode == MANUAL){
        getchar();
        return;
    } else {
        usleep(SC_WAIT * 1000);
    }
    writeOut(TYPE_SALGO, ticket);

}

void initNode(int argc, char *argv[]) {
    if (argc != 4) {
        printWrongUsageError();
        exit(0);
    }
    nodeID = atoi(argv[1]);
    totalNodes = atoi(argv[2]);
    if (nodeID > totalNodes) {
        printWrongUsageError();
    }
    mode = atoi(argv[3]);
    sprintf(mainTag, "MAIN %d> ", nodeID);
    sprintf(receptorTag, "RECEPTOR %d> ", nodeID);

    initSemaphores(&semMaxPetition, &semWantTo, &semPending, &semTicket);
    initMailBoxes(nodeID);
    initReceptor(receptorMain);
}

void printWrongUsageError() {
    printf("Wrong arguments\nUsage: ./main nodeID totalNodes mode (nodeID <= totalNodes)\n");
}

#pragma clang diagnostic pop
