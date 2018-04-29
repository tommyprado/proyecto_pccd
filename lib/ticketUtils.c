#include "../headers/ticketUtils.h"
#include <stdio.h>

int compTickets(ticket ticket1, ticket ticket2) {
    if (ticket1.requestID > ticket2.requestID) {
        return 1;
    }
    if (ticket1.requestID < ticket2.requestID) {
        return -1;
    }
    if (ticket1.requestID == ticket2.requestID) {
        if (ticket1.nodeID > ticket2.nodeID) {
            return 1;
        }
        if (ticket1.nodeID < ticket2.nodeID) {
            return -1;
        }
        return 0;
    }
    return 0;
}

void ticketToString(char *string, ticket ticket) {
    sprintf(string, "[%d, %d, %d]", ticket.requestID, ticket.nodeID, ticket.pid);
}