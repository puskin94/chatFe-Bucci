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
#include "include/userManagement.h"
#include "include/threadDispatcher.h"


#define MSG_LOGIN 'L'
#define MSG_REGLOG 'R'
#define MSG_OK 'O'
#define MSG_ERROR 'E'
#define MSG_SINGLE 'S'
#define MSG_BRDCAST 'B'
#define MSG_LIST 'I'
#define MSG_LOGOUT 'X'


typedef struct {
    char *message[K];
    char *receiver[K];
    char *sender[K];
    int readpos, writepos;
    int count;
    pthread_mutex_t buffMux;
    pthread_cond_t FULL;
    pthread_cond_t EMPTY;
} buffStruct;


buffStruct bufferPC;
buffStruct *BufferPC = &bufferPC;


void *launchThreadDispatcher() {

    char *sender, *receiver, *msg, *sendBuffer, *userName, *logMsg;
    bool go = true, isBrd;
    int receiverId;

    hdata_t *hashUser = (hdata_t *) malloc(sizeof(struct msg_t*));

    sendBuffer = malloc(sizeof(char));

    logMsg = malloc(sizeof(char));
    sender = malloc(sizeof(char));
    receiver = malloc(sizeof(char));
    msg = malloc(sizeof(char));

    initStruct();

    while(go) {

        isBrd = readFromBufferPC(&sender, &receiver, &msg);

        userName = strtok(receiver, ":");


        do {
            receiverId = returnSockId(userName, hashUser);

            if (!isBrd) {
                sendBuffer = realloc(sendBuffer, 6 + strlen(sender) + 1 + strlen(userName) + 2 + strlen(msg));
                sprintf(sendBuffer, "%06zu%s:%s:%s", strlen(sender) + 1 + strlen(userName) + 1 + strlen(msg),
                                                    sender,
                                                    userName,
                                                    msg);
            } else {
                sendBuffer = realloc(sendBuffer, 6 + strlen(sender) + 4 + strlen(msg));
                sprintf(sendBuffer, "%06zu%s:*:%s", strlen(sender) + 3 + strlen(msg),
                                                    sender,
                                                    msg);
            }


            if(send(receiverId , sendBuffer , strlen(sendBuffer), 0) < 0) {
                logMsg = strdup("[!] Cannot send Infos to the client!");
                buildLog(logMsg, 1);
            }


            // scrivo sul log-file i messaggi
            logMsg = realloc(logMsg, strlen(sender) + strlen(userName) + strlen(msg) + 3);
            sprintf(logMsg, "%s:%s:%s", sender, userName, msg);
            buildLog(logMsg, 0);


            // prendo l'username seguente
            userName = strtok(NULL, ":");

        } while (userName != NULL);

    }

    pthread_exit(NULL);
}


bool readFromBufferPC(char **sender, char **receiver, char **msg) {

    bool isBrd = false;

    char *tmpBuff = malloc(sizeof(char));

    pthread_mutex_lock(&BufferPC->buffMux);
    while (BufferPC->count == 0) {
        pthread_cond_wait(&BufferPC->EMPTY, &BufferPC->buffMux);
    }


    *sender = strdup(BufferPC->sender[BufferPC->readpos]);

    // se BufferPC->receiver è diverso da '*', vuol dire
    // che si tratta di un messaggio singolo, altrimenti è un messaggio broadcast
    if (strcmp(BufferPC->receiver[BufferPC->readpos], "*") != 0) {
        *receiver = strdup(BufferPC->receiver[BufferPC->readpos]);
    } else {
        // listUser restituisce una stringa già formattata
        // per l'invio ( richiesta #ls ). Per adattarla allo scopo di questa funzione
        // bisogna utilizzare il suo contenuto dal settimo byte in poi
        listUser(&tmpBuff);
        *receiver = realloc(*receiver, strlen(tmpBuff+6));
        bzero(*receiver, strlen(tmpBuff+6));
        strcpy(*receiver, tmpBuff+6);

        isBrd = true;
    }

    *msg = strdup(BufferPC->message[BufferPC->readpos]);

    BufferPC->readpos = (BufferPC->readpos + 1) % K;
    BufferPC->count--;

    pthread_mutex_unlock(&BufferPC->buffMux);

    free(tmpBuff);
    return isBrd;
}


void writeOnBufferPC(char *msg) {

    int charsRead = 0;
    char *tmpBuff = malloc(sizeof(char) * 3);

    pthread_mutex_lock(&BufferPC->buffMux);
    while (BufferPC->count == K) {
        pthread_cond_wait(&BufferPC->FULL, &BufferPC->buffMux);
    }

    /*
    MESSAGGI
    single:
    S006puskin005mario00004ciao

    broadcast:
    B005mario00004ciao
    */


    // lettura del tipo di messaggio
    charsRead += 1;

    if (msg[0] == MSG_SINGLE) {

        // copio la lunghezza del receiver dentro tmpBuff
        strncpy(tmpBuff, msg + charsRead, 3);
        charsRead += 3;

        // alloco lo spazio necessario per il receiver nella struttura
        BufferPC->receiver[BufferPC->writepos] = malloc(sizeof(char) * atoi(tmpBuff));
        // popolo il campo
        strncpy(BufferPC->receiver[BufferPC->writepos], msg + charsRead, atoi(tmpBuff));
        charsRead += atoi(tmpBuff);

        // pulisco il buffer per evitare dati impuri
        bzero(tmpBuff, 3);

    } else {
        // per come è formattato il messaggio da inviare al client,
        // il destinatario è * ( per i messaggi Broadcast )
        BufferPC->receiver[BufferPC->writepos] = malloc(sizeof(char) * 2);
        // popolo il campo
        strncpy(BufferPC->receiver[BufferPC->writepos], "*", 2);
    }

    // copio la lunghezza del sender dentro tmpBuff
    tmpBuff = realloc(tmpBuff, sizeof(char) * 3);
    strncpy(tmpBuff, msg + charsRead, 3);
    charsRead += 3;
    // alloco lo spazio necessario per il sender nella struttura
    BufferPC->sender[BufferPC->writepos] = malloc(sizeof(char) * atoi(tmpBuff));
    // popolo il campo
    strncpy(BufferPC->sender[BufferPC->writepos], msg + charsRead, atoi(tmpBuff));
    charsRead += atoi(tmpBuff);

    // pulisco il buffer per evitare dati impuri
    bzero(tmpBuff, 3);

    // copio la lunghezza del messaggio dentro tmpBuff
    tmpBuff = realloc(tmpBuff, sizeof(char) * 5);
    strncpy(tmpBuff, msg + charsRead, 5);
    charsRead += 5;
    // alloco lo spazio necessario per il messaggio nella struttura
    BufferPC->message[BufferPC->writepos] = malloc(sizeof(char) * atoi(tmpBuff));
    // popolo il campo

    strncpy(BufferPC->message[BufferPC->writepos], msg + charsRead, atoi(tmpBuff));


    BufferPC->writepos = (BufferPC->writepos + 1) % K;
    BufferPC->count++;


    pthread_cond_signal(&BufferPC->EMPTY);
    pthread_mutex_unlock(&BufferPC->buffMux);


    free(tmpBuff);

}

void initStruct() {
    BufferPC->readpos = 0;
    BufferPC->writepos = 0;
    BufferPC->count = 0;
    pthread_mutex_init(&BufferPC->buffMux, NULL);
    pthread_cond_init(&BufferPC->FULL, NULL);
    pthread_cond_init(&BufferPC->EMPTY, NULL);
}
