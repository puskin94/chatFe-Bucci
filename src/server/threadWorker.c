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
    int lenToAllocate;
    int sock = *(int*)newConn;

    msg_t *msg_T = malloc(sizeof(struct msg_t*));

    // 'pulisco' il buffer
    bzero(buff, sizeof(char));

    // Viene ricevuto il messaggio e controllato quale servizio
    // viene richiesto


    if(read(sock, buff, sizeof(char)) < 0) {

        printf("[!] Error while reading from socket\n");

    } else {

        /* nelle successive linee viene riempita la struttura 'msg_T'.
        ogni campo viene letto dal socket attraverso una read.
        il dato viene inserito in 'char *buff';
        la grandezza di buff viene allocata dinamicamente attraverso
        la funzione 'realloc()'.
        se necessario ( per i campi char * ), la grandezza dei componenti
        della struttura viene allocata dinamicamente attraverso
        la funzione 'malloc()' */

        msg_T->type = buff[0];

        buff = realloc(buff, 3); // adesso può contenere 3 decimali
        read(sock, buff, 3); // leggo da sock la lunghezza del prossimo campo
        len = atoi(buff); // la rendo 'leggibile' ( da char a int )

        lenToAllocate = sizeof(char) * len;
        buff = realloc(buff, lenToAllocate); //adesso buff può contenere char * len
        read(sock, buff, lenToAllocate); // leggo il campo successivo
        msg_T->sender = malloc(lenToAllocate);
        msg_T->sender = buff;


        // qua leggo len e receiver
        buff = realloc(buff, 3); // adesso può contenere 3 decimali
        read(sock, buff, 3); // leggo da sock la lunghezza del prossimo campo
        len = atoi(buff); // la rendo 'leggibile' ( da char a int )

        lenToAllocate = sizeof(char) * len;
        buff = realloc(buff, lenToAllocate); //adesso buff può contenere char * len
        read(sock, buff, lenToAllocate); // leggo il campo successivo
        msg_T->receiver = malloc(lenToAllocate);
        msg_T->receiver = buff;


        // qua leggo len e msg
        // len lo facciamo un po più grande: 5 digit
        buff = realloc(buff, 5); // adesso può contenere 5 decimali
        read(sock, buff, 5); // leggo da sock la lunghezza del prossimo campo
        len = atoi(buff); // la rendo 'leggibile' ( da char a int )
        msg_T->msglen = len;


        lenToAllocate = sizeof(char) * len;
        buff = realloc(buff, lenToAllocate); //adesso buff può contenere char * len
        read(sock, buff, lenToAllocate); // leggo il campo successivo
        msg_T->msg = malloc(lenToAllocate);
        msg_T->msg = buff;

        // differenziazione delle operazioni da eseguire


    }
    free(buff);
    pthread_exit(NULL);
}
