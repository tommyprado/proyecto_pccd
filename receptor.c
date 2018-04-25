#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>

typedef struct
{
    int requestId;
    int nodeId;

} ticket;



ticket receiveRequest (){

}

void updateMaxPetitionID (int petitionId){
}

void protectWantTo (){

}

void saveRequest (ticket ticket){

}

void unprotectWantTo (){

}

void sendReply (int nodeId){

}

int main(int argc, char *argv[]){

    ticket myTicket;
    ticket originTicket;
    int wantTo;
    int petitionId;
//
    while(1) {
        receiveRequest();
        updateMaxPetitionID(petitionId);
        protectWantTo();

        if(wantTo and (myTicket.requestId < originTicket.requestId) ) {
            unprotectWantTo();
            sendReply(myTicket.nodeId);
        }else{
            saveRequest(myTicket);
            unprotectWantTo();
        }

    }






}