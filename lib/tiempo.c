#include "../headers/tiempo.h"
#include <sys/time.h>

long long int tiempoActual(){
    struct timeval timer_usec;
    long long int tiempo;

    if (!gettimeofday(&timer_usec, NULL)) {
        tiempo = ((long long int) timer_usec.tv_sec) * 1000000ll +
                 (long long int) timer_usec.tv_usec;
    }
    else {
        tiempo = -1;
    }
    printf("%lld tiempo inicio ->\n", tiempo);

    return tiempo;
}