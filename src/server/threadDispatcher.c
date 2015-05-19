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
    char type[K];
    pthread_mutex_t buffMux;
    int readpos, writepos;
    int count;
    pthread_cond_t FULL;
    pthread_cond_t EMPTY;
} buffStruct;


buffStruct bufferPC;

buffStruct *BufferPC = &bufferPC;



void *launchThreadDispatcher() {

    char *sender, *receiver, *msg, *sendBuffer;
    bool go = true;
    int receiverId;

    hdata_t *hashUser = (hdata_t *) malloc(sizeof(struct msg_t*));


    sendBuffer = malloc(sizeof(char));

    sender = malloc(sizeof(char));
    receiver = malloc(sizeof(char));
    msg = malloc(sizeof(char));

    initStruct();

    while(go) {
        readFromBufferPC(sender, receiver, msg);
        receiverId = returnSockId(receiver, hashUser);

        sendBuffer = realloc(sendBuffer, 6 + strlen(sender) + 1 + strlen(receiver) + 1 + strlen(msg));
        sprintf(sendBuffer, "%06zu%s:%s:%s", strlen(sender) + 1 + strlen(receiver) + 1 + strlen(msg),
                                            sender,
                                            receiver,
                                            msg);

        if(send(receiverId , sendBuffer , strlen(sendBuffer), 0) < 0) {
            buildLog("[!] Cannot send Infos to the client!", 1);
        }
    }

    pthread_exit(NULL);
}


void readFromBufferPC(char *sender, char *receiver, char *msg) {

    pthread_mutex_lock(&BufferPC->buffMux);
    while (BufferPC->count == 0) {
        pthread_cond_wait(&BufferPC->EMPTY, &BufferPC->buffMux);
    }


    sender = realloc(sender, sizeof(BufferPC->sender[BufferPC->readpos]));
    strcpy(sender, BufferPC->sender[BufferPC->readpos]);

    if (BufferPC->receiver[BufferPC->readpos] != NULL) {
        receiver = realloc(receiver, sizeof(BufferPC->sender[BufferPC->readpos]));
        strcpy(receiver, BufferPC->receiver[BufferPC->readpos]);
    }

    msg = realloc(msg, sizeof(BufferPC->sender[BufferPC->readpos]));
    strcpy(msg, BufferPC->message[BufferPC->readpos]);

    BufferPC->readpos = (BufferPC->readpos + 1) % K;
    BufferPC->count--;

    pthread_mutex_unlock(&BufferPC->buffMux);
}


void writeOnBufferPC(char *msg) {

    int charsRead = 0;
    char *tmpBuff = malloc(sizeof(char) * 3);;

    pthread_mutex_lock(&BufferPC->buffMux);
    while (BufferPC->count == K) {
        pthread_cond_wait(&BufferPC->FULL, &BufferPC->buffMux);
    }


    BufferPC->type[BufferPC->writepos] = msg[0];
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
        BufferPC->receiver[BufferPC->writepos] = malloc(sizeof(char) * strlen("*"));
        // popolo il campo
        strncpy(BufferPC->receiver[BufferPC->writepos], "*", strlen("*"));
    }

    // copio la lunghezza del sender dentro tmpBuff
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

}

void initStruct() {
    BufferPC->readpos = 0;
    BufferPC->writepos = 0;
    BufferPC->count = 0;
}
