#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>


#include "include/utils.h"

/* questo define è un Personal Logout.
Il server invia SEMPRE 6 byte per indicare la lunghezza del messaggio.
Il messaggio in caso di logout non esiste ma il threadListener legge lo stesso
6 Byte.*/
#define P_LOGOUT "00000O"


void *launchThreadListener(void *newConn) {

    bool go = true;
    bool loggedOut = false;

    int sock = *(int*)newConn;
    int len;

    char *buffer = malloc(sizeof(char) * 6);

    while(go && !loggedOut && (read(sock, buffer, sizeof(char) * 6) > 0)) {

        if (strncmp(buffer, P_LOGOUT, 6) != 0) {

            len = atoi(buffer);

            buffer = realloc(buffer, len * sizeof(char));
            read(sock, buffer, len);
            printf("\t%s\n", buffer);

            // pulisco e rialloco il buffer prima di riutilizzarlo
            bzero(buffer, len);
            buffer = realloc(buffer, sizeof(char) * 6);

        } else {
            loggedOut = true;
        }
    }

    pthread_exit(NULL);

}
