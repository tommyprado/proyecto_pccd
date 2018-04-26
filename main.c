#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <zconf.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <pthread.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

#define SEM_MAX_PETITION_NAME "semMaxPetition"
#define SEM_WANT_TO_NAME "semWantTo"
#define NODE_INITIAL_KEY 10000
#define PENDING_SM_COUNT 20000
#define PENDING_SM_ARRAY 30000
#define TYPE_REQUEST 1
#define TYPE_REPLY 2
#define PENDING_REQUESTS_LIMIT 1000000

typedef struct
{
    int nodeID;
    int requestID;

} ticket;

typedef struct {
    long    mtype;
    ticket  ticket;
} messageBuff;

void setWantTo (int value);

void doStuff (int type);

ticket createTicket (int nodeID);

void sendRequests(ticket ticket);

void receiveReply ();

void accessCS (int type);

void replyAllPending ();

void initNode(int argc, char *argv[]);

void printWrongUsageError();

void initReceptor();

void initSemaphores();

void initMailBoxes();

ticket receiveRequest ();

void updateMaxPetitionID (int petitionId);

void protectWantTo ();

void unprotectWantTo ();

void sendReply (ticket ticket);

void saveRequest (ticket ticket);

int compTickets(ticket ticket1, ticket ticket2);

int totalNodes, maxPetition, wantTo, nodeID;
sem_t semMaxPetition, semWantTo;
ticket pendingRequestsArray[PENDING_REQUESTS_LIMIT];
int *pendingRequestsCountPointer;
ticket biggestTicket;

int main(int argc, char *argv[]){
    initNode(argc, argv);
    while (1) {
        doStuff(0);

        setWantTo(1);
        ticket ticket = createTicket(nodeID);
        sendRequests(ticket);
        int countReply = 0;
        while (countReply < totalNodes) {
            receiveReply();
            countReply++;
        }

        accessCS(0);

        setWantTo(0);
        replyAllPending();
    }
}

void * receptorMain(void *arg) {
    while(1) {
        ticket originTicket = receiveRequest();
        updateMaxPetitionID(maxPetition);
        protectWantTo();

        if(wantTo && (compTickets(biggestTicket, originTicket) == 1) ) {
            unprotectWantTo();
            sendReply(biggestTicket);
        }else{
            saveRequest(biggestTicket);
            unprotectWantTo();
        }
    }
}

int compTickets(ticket ticket1, ticket ticket2) {
    if (ticket1.requestID > ticket2.requestID) {
        return 1;
    }
    if (ticket1.requestID < ticket2.requestID) {
        return -1;
    }
    if (ticket1.requestID == ticket2.requestID) {
        if (ticket1.nodeID > ticket2.nodeID) {
            return 1;
        }
        if (ticket1.nodeID < ticket2.nodeID) {
            return -1;
        }
        return 0;
    }
    return 0;
}

ticket receiveRequest (){
    messageBuff message;
    msgrcv(NODE_INITIAL_KEY + nodeID, &message, sizeof(ticket), TYPE_REQUEST, 0);
}

void updateMaxPetitionID (int petitionId){
    sem_wait(&semMaxPetition);

    if(petitionId>maxPetition){
        maxPetition=petitionId;
    }
    sem_post(&semMaxPetition);
}

void protectWantTo (){
    sem_wait(&semWantTo);
}

void saveRequest (ticket ticket){
//    // TODO: Proteger variables
//    pendingRequests *newNode = (pendingRequests *) malloc (sizeof(struct requestNodeList));
//
//    if (newNode == NULL) printf( "No hay memoria disponible!\n");
//    else{
//        newNode->ticket = ticket;
//        newNode->next = NULL;
//
//        if (first == NULL) {
//            printf( "Primer elemento\n");
//            first = newNode ;
//            latest = newNode;
//        }
//        else {
//            latest->next = newNode;
//            latest = newNode;
//        }
//    }
}

void unprotectWantTo (){
    sem_post(&semWantTo);
}

void sendReply (ticket ticket){
    messageBuff message;
    message.mtype = TYPE_REPLY;
    message.ticket = ticket;
    int msg = msgsnd(ticket.nodeID+NODE_INITIAL_KEY, &message, sizeof(ticket), 0);
    if(msg == -1) {
        printf("Error al enviar el ticket\n");
        exit(1);
    }

}

void doStuff (int type){
    if(type == 0){
        printf("\nEsperando salto de linea...\n");
        getchar();
        return;
    }
    usleep(100*1000);
}

void setWantTo (int value){
    sem_wait(&semWantTo);
    wantTo=value;
    sem_post(&semWantTo);
}

ticket createTicket (int nodeID){
    sem_wait(&semMaxPetition);
    maxPetition=maxPetition++;
    ticket myTicket = {.nodeID = nodeID, .requestID = maxPetition};
    sem_post(&semMaxPetition);
    return myTicket;
}

void sendRequests(ticket ticket){
    for(int node = 1; node < totalNodes; node++){
        printf("Enviamos el ticket al nodo %i\n", node);
        if(!node == nodeID){
            messageBuff message;
            message.mtype = TYPE_REQUEST;
            message.ticket = ticket;
            int msg = msgsnd(NODE_INITIAL_KEY + node, &message, sizeof(ticket), 0);
            if(msg == -1) {
                printf("Error al enviar el ticket\n");
                exit(1);
            }
        }
    }
}

void receiveReply (){
    messageBuff message;
    msgrcv(NODE_INITIAL_KEY + nodeID, &message, sizeof(ticket), TYPE_REPLY, 0);
}

void accessCS (int type){
    if(type == 0){
        printf("\nEsperando salto de linea...\n");
        getchar();
        return;
    }
    usleep(100*1000);
}

void replyAllPending (){
    // TODO: replyAllPending
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

    initSemaphores();
    initMailBoxes();
    initReceptor();
}

void initSemaphores() {
    if (sem_init(&semMaxPetition,0,0) && sem_init(&semWantTo, 0, 0)) { // Inicializamos a 0 para que siempre esperen
        printf("Error creating semaphore\n");
        exit(1);
    }
}

void initReceptor() {
    pthread_t receptorThread;
    if(pthread_create(&receptorThread, NULL, receptorMain, NULL)) {
        printf("Error creating thread\n");
        exit(1);
    }
}

void initMailBoxes() {
    int key= nodeID + NODE_INITIAL_KEY;
    int mailbox = msgget(key, 0666 | IPC_CREAT);
    if (mailbox == -1)
    {
        printf("Error buzón\n");
        exit (-1);
    }
}

void printWrongUsageError() {
    printf("Wrong arguments\nUsage: ./main nodeID totalNodes (nodeID <= totalNodes)");
}

#pragma clang diagnostic pop
