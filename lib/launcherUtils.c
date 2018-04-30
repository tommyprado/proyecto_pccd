#include "../headers/launcherUtils.h"
#include "../headers/coms.h"
#include "../headers/tiempo.h"
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>

void sndMsgToLauncher(int type) {
    ticket ticket;
    sndTicketToLauncher(type, ticket);
}

void sndTicketToLauncher(int type, ticket ticket) {
    long long int t = getTimestamp();

    launcherMessage message;
    message.mtype = type;
    message.ticket = ticket;
    message.t = t;

    int msqid = getMsqid(LAUNCHER_QUEUE);
    if(msgsnd(msqid, &message, sizeof(message) - sizeof(long), 0) == -1) {
        printf("Error al invocar 'msgrcv()'.\n");
        exit(0);
    }
}

void getMsgOut(int type) {
    launcherMessage message;
    int msqid = getMsqid(LAUNCHER_QUEUE);
    msgrcv(msqid, &message, sizeof(message) - sizeof(long), type, 0);
}

void writeOut() {
    launcherMessage message;
    int msqid = getMsqid(LAUNCHER_QUEUE);
    msgrcv(msqid, &message, sizeof(message) - sizeof(long), 0, 0);
    if(message.mtype == TYPE_ACCESS_CS){
        FILE * fileSC = fopen("pagos.dat", "w");
        fprintf(fileSC, "%lli 1\n", message.t);
        fclose(fileSC);

    }
    else if(message.mtype == TYPE_EXIT_CS){
        FILE * fileSC = fopen("pagos.dat", "w");
        fprintf(fileSC, "%lli 0\n", message.t);
        fclose(fileSC);
    }
}

void sendReceptorConfirmation () {
    int msqid = getMsqid(LAUNCHER_QUEUE);
    launcherMessage message;
    message.mtype = TYPE_RECEPTOR_CONFIRMATION;
    msgsnd(msqid, &message, sizeof(message) - sizeof(long), 0);
}

void receiveReceptorConfirmation () {
    int msqid = getMsqid(LAUNCHER_QUEUE);
    launcherMessage message;
    msgrcv(msqid, &message, sizeof(message) - sizeof(long), TYPE_RECEPTOR_CONFIRMATION, 0);
}

void initMessageQueue () {
    if (msgget(LAUNCHER_QUEUE, 0666 | IPC_CREAT) == -1)
    {
        printf("Error buzón\n");
        exit (-1);
    }
}
