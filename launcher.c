#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void getConfString(int argc, char **argv, char *returnString);
void printArgumentError();

int main(int argc, char *argv[]) {
    char confString[10000];
    getConfString(argc, argv, confString);
    printf("%s", confString);
    return 0;
}

void getConfString(int argc, char **argv, char *returnString) {
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
    while (1) {
        char buff[200];
        if(fgets(buff, 200, fp) != NULL) {
            strcat(returnString, buff);
        } else {
            fclose(fp);
            break;
        }
    }
}

void printArgumentError() {
    printf("Wrong argument number.\nUsage: ./launcher ./path/to/config/file");
}