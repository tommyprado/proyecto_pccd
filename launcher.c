
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

int getPriority(char *string);

void execProcess(int priority, int node, int nodeCount) ;

void pintar();


int nodeCount = 0;

int processCount = 0;

void escribirTiempos(long long int tiempoInicio, long long int tiempoFin);

void tiempoSeccionCritica(char line[200], long long int *instanteAux, int *scAux, long long int *tiempoSC);

long long int dameTiempoSeccionCritica();



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



    char *nombreFichero="pagos.dat";

    for (int j = 0; j < processCount ; ++j) {
        launcherMessage message= recepcionCualquierMensaje();
        if(message.mtype == TYPE_ACCESS_CS){
            tipoAcceso( nombreFichero,  message);
        }else if(message.mtype == TYPE_EXIT_CS){
            tipoSalida( nombreFichero,  message);
        }
    }

    actualTime = time(0);
    struct tm *tlocal2 = localtime(&actualTime);
    char endTime[128];
    strftime(endTime,128,"%d/%m/%y %H:%M:%S",tlocal2);
    long long int endTimeInSec = getTimestamp();

    printf("Tiempo de inicio: %s. Tiempo de finalización: %s. Han transcurrido %lli microsegundos.\n",initTime, endTime, endTimeInSec-initTimeInSec);
    printf("Se han ejecutado %i procesos en total\n", processCount);
    printf("Todos los procesos pasaron por sección crítica\n");


    printf("%lli tiempo total seccion critica pagos\n", dameTiempoSeccionCritica());
    printf("%lli tiempo total\n",endTimeInSec-initTimeInSec);

    escribirTiempos(endTimeInSec-initTimeInSec,dameTiempoSeccionCritica());

    pintar();

    fclose(fp);// esto que carallo pinta aqui ??

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
        int priority = getPriority(split[2]);
        int count = atoi(split[3]);
        processCount += count;
        for (int i = 0; i < count; ++i) {
            if (fork() == 0) {
                execProcess(priority, node, nodeCount);
            }
        }
    } else if (strcmp(split[0], "-") == 0) {
        int delay = atoi(split[1]);
        usleep((unsigned int) delay * 1000);
    }
}

int getPriority(char *string) {
    if (strcmp(string, "Pag") == 0) {
        return PAGOS;
    }
    if (strcmp(string, "Anu") == 0) {
        return ANULACIONES;
    }
    if (strcmp(string, "Res") == 0) {
        return RESERVAS;
    }
    if (strcmp(string, "Con") == 0) {
        return CONSULTORES;
    }
}

void execProcess(int priority, int node, int nodeCount) {
    char nodeString[100], nodeCountString[100], priorityString[100];
    sprintf(nodeString, "%d", node);
    sprintf(nodeCountString, "%d", nodeCount);
    sprintf(priorityString, "%d", priority);
    execl("./Process", "./Process", priorityString, nodeString, nodeCountString, NULL);
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

    system("gnuplot -persist ../pintargraficas.plot");

}

long long int duracionEjecucion(long long int tiempoInicio){
    return getTimestamp()-tiempoInicio;
}


void escribirTiempos(long long int tiempoTotal, long long int tiempoSeccionCritica){
    long long int  tiempoNoSeccionCritica=tiempoTotal-tiempoSeccionCritica;
    float pTiempoNoSeccionCritica = (float)tiempoNoSeccionCritica / tiempoTotal;
    float pTiempoSeccionCritica=(float)tiempoSeccionCritica/tiempoTotal;
    FILE * fileSC = fopen("porcentajeSCtotal.dat", "w");
    fprintf(fileSC, "ejecucion NoSC SC\n");
    fprintf(fileSC, "1 %f %f\n", pTiempoNoSeccionCritica, pTiempoSeccionCritica);
    fclose(fileSC);

}

long long int dameTiempoSeccionCritica(){

    /*esto donde carallo lo declaro ! */
    long long int tiempoSC=0;
    long long int instanteAux=0;
    int scAux=0;
    char nextLine[LINE_LIMIT];
    int contadorLineas=0;

    FILE *fp;
    fp = fopen ( "pagos.dat" , "r" );
    if (fp==NULL) {fputs ("File error",stderr); exit (1);}
    while (1){
        if((fgets(nextLine, LINE_LIMIT, fp)) != NULL) {
            contadorLineas++;
            tiempoSeccionCritica(nextLine, &instanteAux, &scAux,&tiempoSC);
        } else {
            break;
        }
    }

    fclose ( fp );

    return tiempoSC;
}

void tiempoSeccionCritica(char line[200], long long int *instanteAux, int *scAux, long long int *tiempoSC)
{
    int sc = 0;
    long long int instante = 0;
    char *found;
    char split[20][20];
    int cont = 0;
    while( (found = strsep(&line," \n")) != NULL ) {
        if (strcmp(found,"") != 0) {
            strcpy(split[cont], found);

            if(cont==0){
                instante = atoll(split[cont]);
            }
            if(cont==1){
                sc = atoi(split[cont]);
            }

            cont++;
        }
    }


    if(*scAux==1&&sc==0) {
        *tiempoSC=*tiempoSC+(instante-*instanteAux);
    }
    *instanteAux=instante;
    *scAux=sc;
}