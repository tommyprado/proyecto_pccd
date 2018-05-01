
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

#define DOC_PAGOS "pagos.dat"
#define DOC_ANULACIONES "anulaciones.dat"
#define DOC_PRERESERVAS "prereservas.dat"


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

long long int dameTiempoSeccionCritica(char *documento);

long long int primerInstanteSC(char line[200]);

void imprimeMensajeAFichero();

long long int conseguirPrimerAccesoSC();

long long int conseguirUltimaSalidaSC();

void textoTerminalFinEjecucion( char *initTime,long long int initTimeInSec);


void escribirTiemposProcesos(long long int pagos, long long int anulaciones, long long int prereservas);

int main(int argc, char *argv[]) {
    system("ipcrm --all && killall Process && killall Receptor");
    system("rm pagos.dat");
    system("rm anulaciones.dat");
    system("rm prereservas.dat");


    time_t actualTime = time(0);
    struct tm *tlocal1 = localtime(&actualTime);
    char initTime[128];
    strftime(initTime, 128, "%d/%m/%y %H:%M:%S", tlocal1);
    long long int initTimeInSec = getTimestamp();

    initMessageQueue();
    FILE *fp = getFile(argc, argv);
    char nextLine[LINE_LIMIT];
    while (1) {
        if ((fgets(nextLine, LINE_LIMIT, fp)) != NULL) {
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


    imprimeMensajeAFichero();

    textoTerminalFinEjecucion(  initTime, initTimeInSec);


    long long int tiempoPrimeraSC = conseguirPrimerAccesoSC();
    long long int tiempoUltimaSC = conseguirUltimaSalidaSC();


    long long int tiempoSCPagos = dameTiempoSeccionCritica(DOC_PAGOS);
    long long int tiempoSCAnulaciones = dameTiempoSeccionCritica(DOC_ANULACIONES);
    long long int tiempoSCPrereservas = dameTiempoSeccionCritica(DOC_PRERESERVAS);

    long long int tiempoTotalSC=tiempoSCPagos+tiempoSCAnulaciones+tiempoSCPrereservas;

/* comprobacion tiempos
    printf("tiempo 1º sc %lli, SC pagos %lli, SC prereservas %lli, SC anulaciones %lli, SC total %lli \n",tiempoPrimeraSC,tiempoSCPagos,tiempoSCPrereservas,tiempoSCAnulaciones,tiempoTotalSC);
    printf("ultima SC %lli, diferencia entre 1ºsc y ultima %lli\n",tiempoUltimaSC,tiempoUltimaSC-tiempoPrimeraSC);
*/

    escribirTiempos(tiempoUltimaSC - tiempoPrimeraSC, tiempoTotalSC);
    escribirTiemposProcesos(tiempoSCPagos,tiempoSCAnulaciones,tiempoSCPrereservas);

    pintar();

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

    system("gnuplot -persist ../procesos.plot");

    system("gnuplot -persist ../porcentajeSC.plot");

    system("gnuplot -persist ../scPorProceso.plot");

}

void escribirTiempos(long long int tiempoTotal, long long int tiempoSeccionCritica){
    long long int  tiempoNoSeccionCritica=tiempoTotal-tiempoSeccionCritica;
    float pTiempoNoSeccionCritica = (float)tiempoNoSeccionCritica / tiempoTotal;
    float pTiempoSeccionCritica=(float)tiempoSeccionCritica/tiempoTotal;

    //printf("pTiempoNoSeccionCritica %f  pTiempoSeccionCritica %f tiempoTotal %lli tiempoSeccionCritica %lli\n", pTiempoNoSeccionCritica, pTiempoSeccionCritica, tiempoTotal, tiempoSeccionCritica);


    FILE * fileSC = fopen("porcentajeSCtotal.dat", "w");
    fprintf(fileSC, "ejecucion NoSC SC\n");
    fprintf(fileSC, "1 %f %f\n", pTiempoNoSeccionCritica, pTiempoSeccionCritica);
    fclose(fileSC);

}

void escribirTiemposProcesos(long long int pagos, long long int anulaciones, long long int prereservas) {
    FILE * fileSC = fopen("scPorProceso.dat", "w");
    fprintf(fileSC, "Pagos Anulaciones Prereservas\n");
    fprintf(fileSC, "%lli %lli %lli\n", pagos, anulaciones,prereservas);
    fclose(fileSC);

}

long long int dameTiempoSeccionCritica(char *documento){

    long long int tiempoSC=0;
    long long int instanteAux=0;
    int scAux=0;
    char nextLine[LINE_LIMIT];
    int contadorLineas=0;

    FILE *fp;
    fp = fopen ( documento , "r" );
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

long long int primerInstanteSC(char line[LINE_LIMIT])
{
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
            cont++;
        }
    }
    return instante;
}

void imprimeMensajeAFichero() {
    char *nombreFicheroPagos = DOC_PAGOS;
    char *nombreFicheroAnulaciones = DOC_ANULACIONES;
    char *nombreFicheroPrereservas = DOC_PRERESERVAS;


    for (int j = 0; j < processCount * 2; ++j) {
        launcherMessage message = recepcionCualquierMensaje();


        if (message.ticket.priority == PAGOS) {//si es de tipo pagos
            if (message.mtype == TYPE_ACCESS_CS) {
                tipoAcceso(nombreFicheroPagos, message);
            } else if (message.mtype == TYPE_EXIT_CS) {
                tipoSalida(nombreFicheroPagos, message);
            }
        }
        if (message.ticket.priority == ANULACIONES) {//si es de tipo anulaciones
            if (message.mtype == TYPE_ACCESS_CS) {
                tipoAcceso(nombreFicheroAnulaciones, message);
            } else if (message.mtype == TYPE_EXIT_CS) {
                tipoSalida(nombreFicheroAnulaciones, message);
            }
        }
        if (message.ticket.priority == RESERVAS) {//si es de tipo prereservas
            if (message.mtype == TYPE_ACCESS_CS) {
                tipoAcceso(nombreFicheroPrereservas, message);
            } else if (message.mtype == TYPE_EXIT_CS) {
                tipoSalida(nombreFicheroPrereservas, message);
            }
        }
        if (message.ticket.priority == CONSULTORES) {//si es de tipo consultores
            printf("aun no implementados los consultores");
        }


    }
}

long long int conseguirPrimerAccesoSC(){
    //esto para conseguir el primer instante de acceso a la SC
    char nextLine[LINE_LIMIT];
    long long int tiempoPrimeraSCPagos=0;
    long long int tiempoPrimeraSCAnulaciones=0;
    long long int tiempoPrimeraSCPrereservas=0;

    FILE *fp;
    fp = fopen(DOC_PAGOS, "r");
    if (fp == NULL) {
        perror("File error");
        exit(1);
    }
    if ((fgets(nextLine, LINE_LIMIT, fp)) != NULL) {
        tiempoPrimeraSCPagos = primerInstanteSC(nextLine);
    }
    fclose(fp);

    fp = fopen(DOC_ANULACIONES, "r");
    if (fp == NULL) {
        perror("File error");
        exit(1);
    }
    if ((fgets(nextLine, LINE_LIMIT, fp)) != NULL) {
        tiempoPrimeraSCAnulaciones= primerInstanteSC(nextLine);
    }
    fclose(fp);

    fp = fopen(DOC_PRERESERVAS, "r");
    if (fp == NULL) {
        perror("File error");
        exit(1);
    }
    if ((fgets(nextLine, LINE_LIMIT, fp)) != NULL) {
        tiempoPrimeraSCPrereservas = primerInstanteSC(nextLine);
    }
    fclose(fp);

    if(tiempoPrimeraSCPagos<=tiempoPrimeraSCAnulaciones && tiempoPrimeraSCPagos<=tiempoPrimeraSCPrereservas){
        return tiempoPrimeraSCPagos;
    }
    if(tiempoPrimeraSCAnulaciones<= tiempoPrimeraSCPagos && tiempoPrimeraSCAnulaciones<=tiempoPrimeraSCPrereservas){
        return tiempoPrimeraSCAnulaciones;
    }
    if(tiempoPrimeraSCPrereservas <= tiempoPrimeraSCAnulaciones && tiempoPrimeraSCPrereservas<=tiempoPrimeraSCPagos){
        return tiempoPrimeraSCPrereservas;
    }

}

long long int conseguirUltimaSalidaSC(){
    //esto para conseguir el primer instante de acceso a la SC
    char nextLine[LINE_LIMIT];
    long long int tiempoUltimaSCPagos=0;
    long long int tiempoUltimaSCAnulaciones=0;
    long long int tiempoUltimaSCPrereservas=0;

    FILE *fp;
    fp = fopen(DOC_PAGOS, "r");
    if (fp == NULL) {
        perror("File error");
        exit(1);
    }
    while ((fgets(nextLine, LINE_LIMIT, fp)) != NULL) {
        tiempoUltimaSCPagos = primerInstanteSC(nextLine);
    }
    fclose(fp);



    fp = fopen(DOC_ANULACIONES, "r");
    if (fp == NULL) {
        perror("File error");
        exit(1);
    }
    while ((fgets(nextLine, LINE_LIMIT, fp)) != NULL) {
        tiempoUltimaSCAnulaciones = primerInstanteSC(nextLine);
    }
    fclose(fp);


    fp = fopen(DOC_PRERESERVAS, "r");
    if (fp == NULL) {
        perror("File error");
        exit(1);
    }
    while ((fgets(nextLine, LINE_LIMIT, fp)) != NULL) {
        tiempoUltimaSCPrereservas = primerInstanteSC(nextLine);
    }
    fclose(fp);



    if(tiempoUltimaSCPagos>=tiempoUltimaSCAnulaciones && tiempoUltimaSCPagos>=tiempoUltimaSCPrereservas){
        return tiempoUltimaSCPagos;
    }
    if(tiempoUltimaSCAnulaciones>= tiempoUltimaSCPagos && tiempoUltimaSCAnulaciones>=tiempoUltimaSCPrereservas){
        return tiempoUltimaSCAnulaciones;
    }
    if(tiempoUltimaSCPrereservas >= tiempoUltimaSCAnulaciones && tiempoUltimaSCPrereservas>=tiempoUltimaSCPagos){
        return tiempoUltimaSCPrereservas;
    }

}

void textoTerminalFinEjecucion( char *initTime,long long int initTimeInSec){
    time_t actualTime = time(0);
    struct tm *tlocal2 = localtime(&actualTime);
    char endTime[128];
    strftime(endTime, 128, "%d/%m/%y %H:%M:%S", tlocal2);
    long long int endTimeInSec = getTimestamp();
    printf("_______________________________________________________________________________________________________________________________________________________________________________________\n\n");
    printf("Tiempo de inicio: %s. Tiempo de finalización: %s. Han transcurrido %lli microsegundos.\n", initTime,
           endTime, endTimeInSec - initTimeInSec);
    printf("Se han ejecutado %i procesos en total\n", processCount);
    printf("Todos los procesos pasaron por sección crítica\n");
    printf("_______________________________________________________________________________________________________________________________________________________________________________________\n");
}

