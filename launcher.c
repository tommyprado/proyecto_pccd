
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/msg.h>
#include <zconf.h>
#include "headers/ticketUtils.h"
#include "headers/coms.h"
#include "headers/out.h"

#define LINE_LIMIT 200

void printArgumentError();

FILE * getFile(int argc, char *argv[]) ;

void processLine(char line[LINE_LIMIT]);

void execProcess(int node, int nodeCount);

int main(int argc, char *argv[]) {
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
    //escribir!!!

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
        int nodeCount = atoi(split[1]);
        for (int i = 0; i < nodeCount; ++i) {
            if (fork() == 0){
                execProcess(i + 1, nodeCount);
            }
        }
    }
}

void execProcess(int node, int nodeCount) {
    char command[100];
    sprintf(command, "./Main %d %d 1", node, nodeCount);
    system(command);
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