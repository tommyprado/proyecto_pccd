#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>

#define KEY 1500
#define FILEKEY "/bin/ls"


typedef struct
{
        int nodeID;
        int requestID;

} ticket;

void setWantTo (int value);

void doStuff (int type);

ticket createTicket (int maxPetition, int nodeID);

void sendRequest (ticket ticket);

void receiveReply ();

void accessCS (int type);

void replyAllPending ();

void initNode(int argc, char *argv[]);

void printWrongUsageError();

int nodeID, totalNodes;
sem_t semMaxPetition;

int main(int argc, char *argv[]){
    initNode(argc, argv);
    int countReply;
    int maxPetition;
    int nodeID;
    ticket ticket;
    doStuff(0);
    setWantTo(1);
    createTicket( maxPetition,  nodeID);
    sendRequest(ticket);
    while (countReply<totalNodes) {
        receiveReply();
        countReply++;
    }
    accessCS(0);
    setWantTo(0);
    replyAllPending();
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


}

void printWrongUsageError() {
    printf("Wrong argument number\nUsage: ./main nodeID totalNodes (nodeID <= totalNodes)");
}


void doStuff (int type){

}


void setWantTo (int value){

}

ticket createTicket (int maxPetition, int nodeID){
        sem_wait(&semMaxPetition);
        maxPetition=maxPetition++;
        ticket myTicket = {.nodeID = nodeID, .requestID = maxPetition};
        sem_post(&semMaxPetition);
}

void sendRequest (ticket ticket){
//enviar mensajes a todos los nodos (totalNodes esta global), hacer un for para recorrer todos los buzones van a tener la id
    // 1001 al 1005 si fueran 5 nodos por ejemplo

}

void receiveReply (){

}

void accessCS (int type){
        if(type == 0){
                printf("\nEsperando salto de linea...\n");
                getchar(); //esperando salto de linea
                return;
        }
        usleep(100*1000);
        return;  
}


void replyAllPending (){

}
