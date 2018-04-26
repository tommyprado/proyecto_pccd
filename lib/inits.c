#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include "../headers/inits.h"

#define NODE_INITIAL_KEY 10000

void initMailBoxes(int nodeID) {
    int key= nodeID + NODE_INITIAL_KEY;
    int mailbox = msgget(key, 0666 | IPC_CREAT);
    if (mailbox == -1)
    {
        printf("Error buzón\n");
        exit (-1);
    }
}

void initSemaphores(sem_t *semMaxPetition, sem_t *semWantTo, sem_t *semPending) {
    if (sem_init(semMaxPetition,0,1) && sem_init(semWantTo, 0, 1) && sem_init(semPending, 0, 1)) {
        printf("Error creating semaphore\n");
        exit(1);
    }
}


void initReceptor(void *(*f)(void *arg)) {
    pthread_t receptorThread;
    if(pthread_create(&receptorThread, NULL, (*f), NULL)) {
        printf("Error creating thread\n");
        exit(1);
    }
}