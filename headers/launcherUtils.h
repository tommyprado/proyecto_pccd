#include "ticketUtils.h"

#ifndef PROYECTO_OUT_H
#define PROYECTO_OUT_H

#define TYPE_ACCESS_CS 3
#define TYPE_EXIT_CS 4
#define TYPE_RECEPTOR_CONFIRMATION 5
#define LAUNCHER_QUEUE 283300

void sndMsgOut(int type, ticket ticket);

void writeOut();

void initMessageQueue ();

void receiveReceptorConfirmation ();

void sendReceptorConfirmation ();

#endif //PROYECTO_OUT_H
