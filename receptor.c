#include <sys/msg.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>


#define SEM_MAX_PETITION_NAME "semMaxPetition"
#define SEM_WANT_TO_NAME "semWantTo"
#define NODE_INITIAL_KEY 10000
#define TYPE_REQUEST 1
#define TYPE_REPLY 2

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

typedef struct
{
    int requestId;
    int nodeId;

} ticket;

typedef struct {
    long    mtype;
    ticket  ticket;
} messageBuff;

ticket receiveRequest ();

void updateMaxPetitionID (int petitionId);

void protectWantTo ();

void unprotectWantTo ();

void sendReply (ticket nodeId);

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
            sendReply(biggestTicket);
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

    messageBuff message;
    msgrcv(NODE_INITIAL_KEY + nodeID, &message, sizeof(struct ticket), TYPE_REQUEST, 0);

}

void updateMaxPetitionID (int petitionId){

    sem_wait(semMaxPetition);

    if(petitionId>maxPetition){
        maxPetition=petitionId;
    }

    sem_post(semMaxPetition);


}

void protectWantTo (){
    sem_wait(semWantTo);

}

void saveRequest (ticket ticket){
//añadir ticket a una lista enlazada para poder ir recorriendola enviando los tickets
    //defino la lista
    struct requestsNodeList { /* lista simple enlazada */
        ticket ticket;
        struct requestNodeList *next;
    };

    struct requestsNodeList *first, *latest;


    struct  requestNodeList *newNode;

    first=NULL;
    latest=NULL;//esto ponerlo donde se inicialicen las listas !!

    newNode = (struct requestNodeList *) malloc (sizeof(struct requestNodeList));

    if (newNode==NULL) printf( "No hay memoria disponible!\n");
    else{
        newNode->ticket = ticket;
        newNode->next = NULL;

        if (first==NULL) {
            printf( "Primer elemento\n");
            first = newNode ;
            latest = newNode;
        }
        else {
            /* el que hasta ahora era el último tiene que apuntar al nuevo */
            latest->next = newNode;
            /* hacemos que el nuevo sea ahora el último */
            latest = newNode;
        }
    }





}

void unprotectWantTo (){
    sem_post(semWantTo);
}

void sendReply (ticket ticket){


    messageBuff message;
    message.mtype = TYPE_REPLY;
    message.ticket = ticket;
    int msg = msgsnd(ticket.nodeId+NODE_INITIAL_KEY, &message, sizeof(ticket), 0);
    if(msg == -1) {
        printf("Error al enviar el ticket\n");
        exit(1);
    }

}

#pragma clang diagnostic pop
