#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <zconf.h>
#include <sys/msg.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#define KEY 1500
#define FILEKEY "/bin/ls"

#define SEM_MAX_PETITION_NAME "semMaxPetition"
#define SEM_WANT_TO_NAME "semWantTo"
#define NODE_INITIAL_KEY 10000
#define TYPE_REQUEST 1
#define TYPE_REPLY 2

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

int nodeID, totalNodes, maxPetition;
sem_t* semMaxPetition, *semWantTo;

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

void doStuff (int type){

    if(type == 0){
        printf("\nEsperando salto de linea...\n");
        getchar();
        return;
    }
    usleep(100*1000);

}

void setWantTo (int value){
    sem_wait(semWantTo);
    semWantTo=1;
    sem_post(semWantTo);

}

ticket createTicket (int nodeID){
    sem_wait(semMaxPetition);
    maxPetition=maxPetition++;
    ticket myTicket = {.nodeID = nodeID, .requestID = maxPetition};
    sem_post(semMaxPetition);
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
    msgrcv(NODE_INITIAL_KEY + nodeID, &message, sizeof(struct ticket), TYPE_REPLY, 0);
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
    char maxPetitionString[50];
    char wantToString[50];
    sprintf(maxPetitionString, "%s_%d",SEM_MAX_PETITION_NAME, nodeID);
    sprintf(wantToString, "%s_%d",SEM_WANT_TO_NAME, nodeID);
    semMaxPetition = sem_open(maxPetitionString, O_CREAT, 0644, 1);
    semWantTo = sem_open(wantToString, O_CREAT, 0644, 1);
}

void initReceptor() {
    int pid = fork();
    char id[50];
    sprintf(id, "%d", nodeID);
    if (pid == 0) {
        execl("./receptor", id);
    }
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
    printf("Wrong arguments\nUsage: ./main nodeID totalNodes (nodeID <= totalNodes)");
}

#pragma clang diagnostic pop
