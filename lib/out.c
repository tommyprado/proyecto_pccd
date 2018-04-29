#include <sys/msg.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include "../headers/out.h"
#include "../headers/coms.h"
#include "../headers/tiempo.h"


void sndMsgOut(int type, ticket ticket) {


    long long int t = tiempoActual();

    printMessage message;
    message.mtype = type;
    message.ticket = ticket;
    message.t = t;

    int msqid = getMsqid(WRITE_OUT_QUEUE);
    if(msgsnd(msqid, &message, sizeof(ticket)+sizeof(t), 0) == -1) {
        printf("Error al invocar 'msgrcv()'.\n");
        exit(0);
    }
}

void writeOut() {
    printMessage message;
    int msqid = getNodeReplyMsqid(WRITE_OUT_QUEUE);
    msgrcv(msqid, &message, sizeof(ticket), 0, 0);
    if(message.mtype == TYPE_ENTRO){
        FILE * fileSC = fopen("pagos.dat", "w");
        fprintf(fileSC, "%lli 1\n", message.t);
        fclose(fileSC);

    }
    else if(message.mtype == TYPE_SALGO){
        FILE * fileSC = fopen("pagos.dat", "w");
        fprintf(fileSC, "%lli 0\n", message.t);
        fclose(fileSC);
    }
}

void initMessageQueue () {
    int msqid = msgget(WRITE_OUT_QUEUE, 0666 | IPC_CREAT);
    if (msqid == -1)
    {
        printf("Error buzón\n");
        exit (-1);
    }
}
