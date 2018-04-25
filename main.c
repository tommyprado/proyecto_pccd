#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#define KEY 1500
#define FILEKEY "/bin/ls"


typedef struct
{


} ticket;

void setWantTo (int value);

void doStuff (int type);

ticket createTicket (int maxPetition, int nodeID);

void sendRequest (ticket ticket);

void receiveReply ();

void accessCS (int type);

void replyAllPending ();

int main(int argc, char *argv[]){
    int countReply;
    int maxPetition;
    int nodeID;
    int totalNodes;// numero total de nodos en el sistema
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


void doStuff (int type){
//creo el buzon
    key_t clave;
    msgsz = sizeof (struct ticket) - sizeof (long int);
    clave = ftok (FILEKEY, KEY);
    if (clave == (key_t)-1) {
        printf("\nERROR clave\n");
        exit (-1);

    }else{
        int buzon = msgget(clave, 0777 | IPC_CREAT);
        if (buzon == -1) {
            printf("\nERROR buzon\n");
            exit (-1);
        }
    }
}


void setWantTo (int value){

}

ticket createTicket (int maxPetition, int nodeID){


}

void sendRequest (ticket ticket){

}

void receiveReply (){// no deberia devolver 1 si alguien responde ¿?

}

void accessCS (int type){

}


void replyAllPending (){

}