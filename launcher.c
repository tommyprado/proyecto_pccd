
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/msg.h>
#include <zconf.h>
#include <sys/time.h>
#include "headers/ticketUtils.h"
#include "headers/coms.h"
#include "headers/out.h"

#define LINE_LIMIT 200

#define LAUNCHER_TAG "LAUNCHER>"

void printArgumentError();

FILE * getFile(int argc, char *argv[]) ;

void processLine(char line[LINE_LIMIT]);

void execReceptor(int node);

void execProcess(int node, int nodeCount);

int nodeCount = 0;

int main(int argc, char *argv[]) {

    long long int tiempoInicio=iniciarReloj();

    initMessageQueue();
    FILE *fp = getFile(argc, argv);
    char nextLine[LINE_LIMIT];
    while (1) {
        if((fgets(nextLine, LINE_LIMIT, fp)) != NULL) {
            processLine(nextLine);
        } else {
            break;
        }
    }
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
    } else if (strcmp(split[0], "+") == 0) {
        int node = atoi(split[1]);
//        int type = convertType(split[2]);
        int count = atoi(split[3]);
        for (int i = 0; i < count; ++i) {
            if (fork() == 0) {
                execProcess(node, nodeCount);
            }
        }
        printf("%sLanzando %d procesos\n", LAUNCHER_TAG, count);

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

long long int iniciarReloj(){
    struct timeval timer_usec;
    long long int tiempoInicio;

    if (!gettimeofday(&timer_usec, NULL)) {
        tiempoInicio = ((long long int) timer_usec.tv_sec) * 1000000ll +
                       (long long int) timer_usec.tv_usec;
    }
    else {
        tiempoInicio = -1;
    }
    printf("%lld tiempo inicio ->\n", tiempoInicio);

    return tiempoInicio;
}


long long int duracionEjecucion(long long int tiempoInicio){

    long long int tiempoFin;
    struct timeval timer_usec;

    if (!gettimeofday(&timer_usec, NULL)) {
        tiempoFin = ((long long int) timer_usec.tv_sec) * 1000000ll +
                       (long long int) timer_usec.tv_usec;
    }
    else {
        tiempoFin = -1;
    }


    return tiempoFin-tiempoInicio;
}