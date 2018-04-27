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


#define RECEPTOR_TAG "RECEPTOR> "
#define MAIN_TAG "MAIN> "

#define ENTRO 1

typedef struct {
    long    mtype;
    ticket  ticket;
    __suseconds_t t;


} messageBuff;


void setWantTo (int value);

void initNode(int argc, char *argv[]);

void printWrongUsageError();

void updateMaxPetitionID (int petitionId);

void saveRequest (ticket ticket);

void doStuff(int type) ;

void accessCS (int type,ticket ticket);

int totalNodes, maxPetition, wantTo, nodeID;
sem_t semMaxPetition, semWantTo, semPending, semTicket;
ticket pendingRequestsArray[PENDING_REQUESTS_LIMIT];
int pendingRequestsCount;
ticket myTicket;

int main(int argc, char *argv[]){
    initNode(argc, argv);
    while (1) {
        printf("%sMoviendo papeles...\n", MAIN_TAG);
        doStuff(0);
        printf("%sIntentando acceder a la sección crítica...\n", MAIN_TAG);
        setWantTo(1);
        sem_wait(&semTicket);
        myTicket = createTicket(nodeID, &maxPetition, &semMaxPetition);
        sendRequests(myTicket, nodeID, totalNodes);
        sem_post(&semTicket);
        int countReply = 1;
        while (countReply < totalNodes) {
            receiveReply(nodeID);
            printf("%sReply recibido\n", MAIN_TAG);
            countReply++;
        }
        printf("%sAccediendo a la sección crítica...\n", MAIN_TAG);
        accessCS(0,myTicket);
        printf("%sFuera de la sección crítica\n", MAIN_TAG);
        setWantTo(0);
        printf("%sRespondiendo a Requests pendientes...\n", MAIN_TAG);
        replyAllPending(&semPending, &pendingRequestsCount, pendingRequestsArray, nodeID);
    }
}

void * receptorMain(void *arg) {
    while(1) {
        printf("%sEsperando a recibir mensaje\n", RECEPTOR_TAG);
        ticket originTicket = receiveRequest(nodeID);
        printf("%sMensaje recibido\n", RECEPTOR_TAG);
        updateMaxPetitionID(maxPetition);
        sem_wait(&semWantTo);
        sem_wait(&semTicket);
        if(!wantTo || wantTo && (compTickets(myTicket, originTicket) == 1) ) { // myTicket > originTicket?
            sem_post(&semTicket);
            sem_post(&semWantTo);
            printf("%sEnviando reply\n", RECEPTOR_TAG);
            sendReply(originTicket);
        } else{
            sem_post(&semTicket);
            printf("%sGuardando request\n", RECEPTOR_TAG);
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

void doStuff(int type) {
    if(type == 0){
        getchar();
        return;
    } else {
        usleep(100*1000);
    }
}

void funcion (int tipo, ticket ticket) {

    struct timeval tv;
    gettimeofday(&tv,NULL);
    __suseconds_t t = tv.tv_usec;
    messageBuff message;
    message.mtype = tipo;
    message.ticket = ticket;
    message.t = t;

    if(msgsnd(COMMON_MAILBOX_KEY, &message, sizeof(ticket)+sizeof(t), 0) == -1) {
        printf("Error al invocar 'msgrcv()'.\n");
        exit(0);
    }

}


void accessCS (int type,ticket ticket){
    funcion(TYPE_ENTRO,ticket);
    if(type == 0){
        getchar();
        return;
    } else {
        usleep(100*1000);
    }
    funcion(TYPE_SALGO,ticket);

}

void initNode(int argc, char *argv[]) {
    if (argc != 3) {
        printWrongUsageError();
        exit(0);
    }
    nodeID = atoi(argv[1]);
    totalNodes = atoi(argv[2]);
    if (nodeID > totalNodes) {
        printWrongUsageError();
    }

    initSemaphores(&semMaxPetition, &semWantTo, &semPending, &semTicket);
    initMailBoxes(nodeID);
    initReceptor(receptorMain);
}

void printWrongUsageError() {
    printf("Wrong arguments\nUsage: ./main nodeID totalNodes (nodeID <= totalNodes)\n");
}

#pragma clang diagnostic pop
