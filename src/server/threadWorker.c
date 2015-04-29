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

#include "include/chat-server.h"
#include "include/utils.h"
#include "include/hash.h"

#define MSG_LOGIN 'L'
#define MSG_REGLOG "R"
#define MSG_OK 'O'
#define MSG_ERROR 'E'
#define MSG_SINGLE 'S'
#define MSG_BRDCAST 'B'
#define MSG_LIST 'I'
#define MSG_LOGOUT 'X'

typedef struct {
    char type;
    char *sender;
    char *receiver;
    unsigned int msglen;
    char *msg;
} msg_t;


void *launchThreadWorker(void *newConn) {

    //pthread_mutex_t mux = PTHREAD_MUTEX_INITIALIZER;

    char *buff = malloc(sizeof(char)); // la dimensione iniziale è quella di un char
                                        // ovvero il primo token da leggere

    int len;

    int sock = *(int*)newConn;

    msg_t *msg_T = malloc(sizeof(struct msg_t*));

    bzero(buff, sizeof(char));

    // Viene ricevuto il messaggio e controllato quale servizio
    // viene richiesto

/*    if(read(sock, buff, sizeof(buff)) < 0) {

        printf("[!] Error while reading from socket\n");

    } else {*/

    if(read(sock, buff, sizeof(char)) < 0) {

        printf("[!] Error while reading from socket\n");

    } else {
        printf("type --> %s\n", buff);
        //strcpy(msg_T->type, buff);

        // msg == type|len|sender|len|receiver|msglen|msg

        // qua leggo len e sender

        buff = realloc(buff, 3); // adesso può contenere 3 decimali
        read(sock, buff, 3); // leggo da sock la lunghezza del prossimo campo
        len = atoi(buff); // la rendo 'leggibile' ( da char a int )

        printf("senderLen --> %d\n",len);


        buff = realloc(buff, sizeof(char) * len); //adesso buff può contenere char * len
        read(sock, buff, sizeof(char) * len); // leggo il campo successivo
        printf("sender --> %s\n",buff);
        // qua leggo len e receiver


        buff = realloc(buff, 3); // adesso può contenere 3 decimali
        read(sock, buff, 3); // leggo da sock la lunghezza del prossimo campo
        len = atoi(buff); // la rendo 'leggibile' ( da char a int )

        printf("receiverLen --> %d\n",len);


        buff = realloc(buff, sizeof(char) * len); //adesso buff può contenere char * len
        read(sock, buff, sizeof(char) * len); // leggo il campo successivo
        printf("receiver --> %s\n",buff);
        // qua leggo len e msg

        // len lo facciamo un po più grande: 5 digit

        buff = realloc(buff, 5); // adesso può contenere 5 decimali
        read(sock, buff, 5); // leggo da sock la lunghezza del prossimo campo
        len = atoi(buff); // la rendo 'leggibile' ( da char a int )

        printf("msgLen --> %d\n",len);


        buff = realloc(buff, sizeof(char) * len); //adesso buff può contenere char * len
        read(sock, buff, sizeof(char) * len); // leggo il campo successivo
        printf("msg --> %s\n", buff);


        /*action = strtok(cmdReceived, ":");

        if (strcmp(action, MSG_LOGIN) == 0) {
                        // viene richesto il LOGIN
            userName = strtok(NULL, " ");
                        // ricerca di 'username' nella hashtable
                        printf("%s\n",userName);

        } else if (strcmp(action, MSG_REGLOG) == 0) {
                        // viene richiesta la registrazione ed il login
            fullName = strtok(NULL, ":");
            mail = strtok(NULL, ":");
            userName = strtok(NULL, ":");

            printf("%s --> %s --> %s\n",fullName, mail, userName );

        } else if (strcmp(action, MSG_LIST) == 0) {
                        // viene richiesta la lista degli utenti online

        } else if (strcmp(action, MSG_LOGOUT) == 0) {
                        // viene richiesta la registrazione ed il login
        }*/

    }

    return NULL;
}
