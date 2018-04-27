
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/msg.h>
#include "headers/ticketUtils.h"
#include "headers/coms.h"

#define COMMON_MAILBOX_KEY 283300
#define TYPE_ENTRO 3
#define TYPE_SALGO 4
#define LINE_LIMIT 200

void printArgumentError();

FILE * getFile(int argc, char *argv[]) ;

char *getNextLine(FILE *fp, char *nextLine) ;

void processLine(char line[LINE_LIMIT]);

void escribir();

int main(int argc, char *argv[]) {
    FILE *fp = getFile(argc, argv);
    while (1) {
        char nextLine[LINE_LIMIT];
        if(getNextLine(fp, nextLine) != NULL) {
            processLine(nextLine);
        } else {
            break;
        }
    }
    fclose(fp);
    //escribir!!!

    escribir();


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
        printf("%d\n", nodeCount);
    }
}

void escribir() {

    messageBuff message;

    msgrcv(COMMON_MAILBOX_KEY, &message, sizeof(ticket), TYPE_ENTRO, 0);
    if(message.mtype==TYPE_ENTRO){
        FILE * fileSC = fopen("pagos.dat", "w");
          fprintf(fileSC, "%li 1\n", message.t);

    }
    msgrcv(COMMON_MAILBOX_KEY, &message, sizeof(ticket), TYPE_SALGO, 0);
    if(message.mtype==TYPE_SALGO){
        FILE * fileSC = fopen("pagos.dat", "w");
          fprintf(fileSC, "%li 0\n", message.t);

    }

}


char *getNextLine(FILE *fp, char *nextLine) {
    return fgets(nextLine, LINE_LIMIT, fp);
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