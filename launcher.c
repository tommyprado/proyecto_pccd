
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

void getConfString(int argc, char **argv, char *returnString);
void printArgumentError();

FILE * getFile(int argc, char *argv[]) ;

char *getNextLine(FILE *fp, char *nextLine) ;

int main(int argc, char *argv[]) {
    FILE *fp = getFile(argc, argv);
    while (1) {
        char nextLine[200];
        if(getNextLine(fp, nextLine) != NULL) {

        } else {
            break;
        }
    }
    fclose(fp);
    return 0;
}

char *getNextLine(FILE *fp, char *nextLine) {
    return fgets(nextLine, 200, fp);
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


bool startsWith(const char *a, const char *b){
    return strncmp(a, b, strlen(b)) == 0;
}