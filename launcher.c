
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/msg.h>
#include <unistd.h>
#include <time.h>
#include "headers/ticketUtils.h"
#include "headers/coms.h"
#include "headers/launcherUtils.h"
#include "headers/tiempo.h"

#define LINE_LIMIT 200

#define LAUNCHER_TAG "LAUNCHER> "

void printArgumentError();

FILE * getFile(int argc, char *argv[]) ;

void processLine(char line[LINE_LIMIT]);

void execReceptor(int node);

void execProcess(int node, int nodeCount);

int nodeCount = 0;

int processCount = 0;


int main(int argc, char *argv[]) {
    system("ipcrm --all && killall Process && killall Receptor");


    time_t actualTime = time(0);
    struct tm *tlocal1 = localtime(&actualTime);
    char initTime[128];
    strftime(initTime,128,"%d/%m/%y %H:%M:%S",tlocal1);
    long long int initTimeInSec = getTimestamp();

    initMessageQueue();
    FILE *fp = getFile(argc, argv);
    char nextLine[LINE_LIMIT];
    while (1) {
        if((fgets(nextLine, LINE_LIMIT, fp)) != NULL) {
            if (strcmp(nextLine, "\n") != 0) {
                processLine(nextLine);
            }
        } else {
            break;
        }
    }
    for (int i = 0; i < processCount; ++i) {
        getMsgOut(TYPE_PROCESS_FINISHED);
    }

    actualTime = time(0);
    struct tm *tlocal2 = localtime(&actualTime);
    char endTime[128];
    strftime(endTime,128,"%d/%m/%y %H:%M:%S",tlocal2);
    long long int endTimeInSec = getTimestamp();

    printf("Tiempo de inicio: %s. Tiempo de finalización: %s. Han transcurrido %lli microsegundos.\n",initTime, endTime, endTimeInSec-initTimeInSec);
    printf("Se han ejecutado %i procesos en total\n", processCount);
    printf("Todos los procesos pasaron por sección crítica\n");
    fclose(fp);
    writeOut();
    return 0;
}

void processLine(char line[LINE_LIMIT]) {
    char *found;
    char split[20][20];
    int cont = 0;
    while( (found = strsep(&line," \n")) != NULL ) {
        if (strcmp(found, "") != 0) {
            strcpy(split[cont], found);
            cont++;
        }
    }

    if (strcmp(split[0], "#") == 0) {
        nodeCount = atoi(split[1]);
        for (int i = 0; i < nodeCount; ++i) {
            if (fork() == 0){
                execReceptor(i + 1);
            }
        }
        printf("%sLanzando %d receptores\n", LAUNCHER_TAG, nodeCount);
        for (int j = 0; j < nodeCount; ++j) {
            receiveReceptorConfirmation();
        }
    } else if (strcmp(split[0], "+") == 0) {
        int node = atoi(split[1]);
//        int type = convertType(split[2]);
        int count = atoi(split[3]);
        processCount += count;
        for (int i = 0; i < count; ++i) {
            if (fork() == 0) {
                execProcess(node, nodeCount);
            }
        }
    } else if (strcmp(split[0], "-") == 0) {
        int delay = atoi(split[1]);
        usleep((unsigned int) delay * 1000);
    }
}

void execProcess(int node, int nodeCount) {
    char nodeString[100], nodeCountString[100];
    sprintf(nodeString, "%d", node);
    sprintf(nodeCountString, "%d", nodeCount);
    execl("./Process", "./Process", nodeString, nodeCountString, NULL);
}

void execReceptor(int node) {
    char nodeString[100];
    sprintf(nodeString, "%d", node);
    execl("./Receptor", "./Receptor", nodeString, NULL);
}

FILE * getFile(int argc, char *argv[]) {
    if (argc != 2) {
        printArgumentError();
        exit(0);
    }
    FILE *fp;
    char *path = argv[1];

    fp = fopen(path, "r");
    if (fp == NULL) {
        perror("Error opening configuration file");
    }
    return fp;
}

void printArgumentError() {
    printf("Wrong argument number.\nUsage: ./launcher ./path/to/config/file");
}


void pintar(){
    FILE * ventanaGnuplot = popen ("gnuplot -persist", "w");
    fprintf(ventanaGnuplot, "%s \n", "load \"pintargraficas.plot\"");
    int fclose (FILE *ventanaGnuplot);
}

