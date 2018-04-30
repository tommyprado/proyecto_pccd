#include "ticketUtils.h"
#include "coms.h"

#ifndef PROYECTO_OUT_H
#define PROYECTO_OUT_H

#define TYPE_ACCESS_CS 3
#define TYPE_EXIT_CS 4
#define TYPE_RECEPTOR_CONFIRMATION 5
#define TYPE_PROCESS_FINISHED 6
#define LAUNCHER_QUEUE 283300

void sndTicketToLauncher(int type, ticket ticket);

void sndMsgToLauncher(int type);


void initMessageQueue ();

void receiveReceptorConfirmation ();

void sendReceptorConfirmation ();

void getMsgOut(int type);

launcherMessage recepcionCualquierMensaje();

void tipoAcceso(char *nombreFichero, launcherMessage message);

void tipoSalida(char *nombreFichero, launcherMessage message);


#endif //PROYECTO_OUT_H
