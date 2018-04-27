#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include "../headers/inits.h"

#define NODE_INITIAL_KEY 10000
#define COMMON_MAILBOX_KEY 283300

void initMailBoxes(int nodeID) {
    int key= nodeID + NODE_INITIAL_KEY;
    int msqid = msgget(key, 0666 | IPC_CREAT);
    if (msqid == -1)
    {
        printf("Error buzón\n");
        exit (-1);
    }


     msqid = msgget(COMMON_MAILBOX_KEY, 0666 | IPC_CREAT);
    if (msqid == -1)
    {
        printf("Error buzón\n");
        exit (-1);
    }


}

void initSemaphores(sem_t *semMaxPetition, sem_t *semWantTo, sem_t *semPending, sem_t *semTicket) {
    int err = sem_init(semWantTo, 0, 1);
    err += sem_init(semPending, 0, 1);
    err += sem_init(semMaxPetition, 0, 1);
    err += sem_init(semTicket, 0, 1);
    if (err) {
        printf("Error inicializando semáforos\n");
        exit(1);
    }
}


void initReceptor(void *(*f)(void *arg)) {
    pthread_t receptorThread;
    if(pthread_create(&receptorThread, NULL, (*f), NULL)) {
        printf("Error creando el hilo\n");
        exit(1);
    }
}