#include <sys/msg.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include "../headers/out.h"
#include "../headers/coms.h"

void sndMsgOut(int type, ticket ticket) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    __suseconds_t t = tv.tv_usec;

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
    int msqid = getMsqid(WRITE_OUT_QUEUE);
    msgrcv(msqid, &message, sizeof(ticket), TYPE_ENTRO, 0);
    if(message.mtype==TYPE_ENTRO){
        FILE * fileSC = fopen("pagos.dat", "w");
        fprintf(fileSC, "%li 1\n", message.t);

    }
    msgrcv(msqid, &message, sizeof(ticket), TYPE_SALGO, 0);
    if(message.mtype==TYPE_SALGO){
        FILE * fileSC = fopen("pagos.dat", "w");
        fprintf(fileSC, "%li 0\n", message.t);
    }
}
