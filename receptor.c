#include <sys/msg.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#define SEM_MAX_PETITION_NAME "semMaxPetition"
#define SEM_WANT_TO_NAME "semWantTo"
#define NODE_INITIAL_KEY 10000

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
typedef struct
{
    int requestId;
    int nodeId;

} ticket;

ticket receiveRequest ();

void updateMaxPetitionID (int petitionId);

void protectWantTo ();

void unprotectWantTo ();

void sendReply (int nodeId);

void saveRequest (ticket ticket);

void initReceptor(int i, char *pString[]);

void initSemaphores();

void printWrongUsageError() ;

void initMailBoxes() ;

int nodeID;

// Shared variables
int maxPetition, wantTo;
ticket biggestTicket;

// Semaphores
sem_t *semMaxPetition, *semWantTo;

int main(int argc, char *argv[]){
    initReceptor(argc, argv);
    while(1) {
        ticket originTicket = receiveRequest();
        updateMaxPetitionID(maxPetition);
        protectWantTo();

        if(wantTo && (biggestTicket.requestId < originTicket.requestId) ) {
            unprotectWantTo();
            sendReply(biggestTicket.nodeId);
        }else{
            saveRequest(biggestTicket);
            unprotectWantTo();
        }

    }
}

void initReceptor(int argc, char *argv[]) {
    if (argc != 2) {
        printWrongUsageError();
    }
    nodeID = atoi(argv[1]);
    initSemaphores();
    initMailBoxes();
}

void initMailBoxes() {
    int key= nodeID + NODE_INITIAL_KEY;
    int mailbox = msgget(key, 0644 | IPC_CREAT);
    if (mailbox == -1)
    {
        printf("Error buzón\n");
        exit (-1);
    }
}

void printWrongUsageError() {
    printf("Wrong arguments\nUsage: ./receptor nodeID");
}

void initSemaphores() {
    char maxPetitionString[50];
    char wantToString[50];
    sprintf(maxPetitionString, "%s_%d",SEM_MAX_PETITION_NAME, nodeID);
    sprintf(wantToString, "%s_%d",SEM_WANT_TO_NAME, nodeID);
    semMaxPetition = sem_open(maxPetitionString, 0);
    semWantTo = sem_open(wantToString, 0);
}


ticket receiveRequest (){

}

void updateMaxPetitionID (int petitionId){
}

void protectWantTo (){

}

void saveRequest (ticket ticket){

}

void unprotectWantTo (){

}

void sendReply (int nodeId){

}
#pragma clang diagnostic pop