#ifndef PROYECTO_INITS_H
#define PROYECTO_INITS_H

void initMailBoxes(int nodeID);

void initSemaphores(sem_t *semMaxPetition, sem_t *semWantTo, sem_t *semPending);

void initReceptor(void *(*f)(void *arg));

#endif //PROYECTO_INITS_H
