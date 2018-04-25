#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <zconf.h>
#include <sys/msg.h>

#define KEY 1500
#define FILEKEY "/bin/ls"

#define SEM_MAX_PETITION_NAME "semMaxPetition"
#define SEM_WANT_TO_NAME "semWantTo"


typedef struct
{
    int nodeID;
    int requestID;

} ticket;

void setWantTo (int value);

void doStuff (int type);

ticket createTicket (int nodeID);

void sendRequest (ticket ticket);

void receiveReply ();

void accessCS (int type);

void replyAllPending ();

void initNode(int argc, char *argv[]);

void printWrongUsageError();

void initReceptor();

int nodeID, totalNodes, maxPetition;
sem_t* semMaxPetition, *semWantTo;

int IDColaMensajes;

int main(int argc, char *argv[]){
    initNode(argc, argv);
    int countReply;
    int maxPetition;
    int nodeID;
    int totalNodes;
    ticket ticket;
    doStuff(0);
    setWantTo(1);
    createTicket(nodeID);
    sendRequest(ticket);
    while (countReply < totalNodes) {
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

    char maxPetitionString[50];
    char wantToString[50];
    sprintf(maxPetitionString, "%s_%d",SEM_MAX_PETITION_NAME, nodeID);
    sprintf(wantToString, "%s_%d",SEM_WANT_TO_NAME, nodeID);
    semMaxPetition = sem_open(maxPetitionString, O_CREAT, 0644, 1);
    semWantTo = sem_open(wantToString, O_CREAT, 0644, 1);

    initReceptor();

    int clave= nodeID + 10000;
    int buzon = msgget(clave, 0644 | IPC_CREAT);
    if (buzon == -1)
    {
        printf("Error buzon\n");
        exit (-1);
    }
}

void initReceptor() {
    int pid = fork();
    char id[50];
    sprintf(id, "%d", nodeID);
    if (pid == 0) {
        execl("./receptor", id);
    }
}


void printWrongUsageError() {
    printf("Wrong argument number\nUsage: ./main nodeID totalNodes (nodeID <= totalNodes)");
}


void doStuff (int type){

}


void setWantTo (int value){

}

ticket createTicket (int nodeID){
    sem_wait(semMaxPetition);
    maxPetition=maxPetition++;
    ticket myTicket = {.nodeID = nodeID, .requestID = maxPetition};
    sem_post(semMaxPetition);
    return myTicket;
}

void sendRequest (ticket ticket){
    for(int mailboxDestiny = 100001; mailboxDestiny < 100000+totalNodes; mailboxDestiny++){
        printf("Enviamos el ticket al nodo %i\n", nodoDestino);
        if(!mailboxDestiny == 10000+nodeID){
            int msg = msgsnd(mailboxDestiny, &ticket, (sizeof(ticket) - sizeof(long)), 0);
            if(msg == -1) {
                printf("Error al enviar el ticket\n");
                exit(1);
            }
        }
    }
}

void receiveReply (){
//tengo un buzon y recibo de cualquier nodo algo



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
