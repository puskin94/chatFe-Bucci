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
#define MSG_REGLOG 'R'
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

static hash_t HASH_TABLE;

bool registerNewUser(char *msg, hdata_t *user);
bool loginUser(char *user, hdata_t *bs, int sock);
char *listUser(hdata_t *online);


void *launchThreadWorker(void *newConn) {

    pthread_mutex_t mux = PTHREAD_MUTEX_INITIALIZER;


    char *buff = malloc(sizeof(char)); // la dimensione iniziale è quella di un char
                                        // ovvero il primo token da leggere

    char *tmpBuff = malloc(sizeof(char));

    // questa variabile verrà riempita dopo la registrazione con
    // il nome dell'utente che dovrà effettuare il login
    char *userName;

    int lenToAllocate;
    int sock = *(int*)newConn;

    bool go = true;

    char logMsg[256];


    msg_t *msg_T = malloc(sizeof(struct msg_t*));

    hdata_t *user = (hdata_t *) malloc(sizeof(hdata_t));
    hdata_t *bs = (hdata_t *) malloc(sizeof(hdata_t));

    // 'pulisco' il buffer
    // 'pulisco' l'array dei log
    bzero(buff, sizeof(char));
    bzero(logMsg, 256);


    HASH_TABLE = CREAHASH();

    // Viene ricevuto il messaggio e controllato quale servizio
    // viene richiesto


    if(read(sock, buff, sizeof(char)) < 0) {

        printf("[!] Error while reading from socket\n");

    } else {

        while (go) {

            /* nelle successive linee viene riempita la struttura 'msg_T'.
            ogni campo viene letto dal socket attraverso una read.
            il dato viene inserito in 'char *buff';
            la grandezza di buff viene allocata dinamicamente attraverso
            la funzione 'realloc()'.
            se necessario ( per i campi char * ), la grandezza dei componenti
            della struttura viene allocata dinamicamente attraverso
            la funzione 'malloc()' */

            // -------> REGOLE DIPENDENTI DALLA CONSEGNA <------- //

            /* i comandi passati dal client di tipo
            MSG_LOGIN, MSG_REGLOG, MSG_LOGOUT, MSG_LIST, MSG_BRDCAST
            non hanno i campi sender & receiver.
            il comando MSG_SINGLE ha solo il campo receiver.
            in poche parole ( da lato del server ), il campo sender messo nella
            struttura è SEMPRE vuoto
            per i messaggi di tipo MSG_LOGOUT & MSG_LIST il campo msg è vuoto */


            msg_T->type = buff[0];

            // i 3 decimali devo leggerli lo stesso per consumare il socket

            buff = realloc(buff, 3); // adesso può contenere 3 decimali
            read(sock, buff, 3); // leggo da sock la lunghezza del prossimo campo
            msg_T->sender = malloc(sizeof(""));
            msg_T->sender = strdup("");

            // qua leggo len e receiver
            buff = realloc(buff, 3); // adesso può contenere 3 decimali
            read(sock, buff, 3); // leggo da sock la lunghezza del prossimo campo

            // se il messaggio è di tipo MSG_SINGLE
            // il campo 'receiver' è pieno, quindi devo leggerlo dal socket

            if (msg_T->type == MSG_SINGLE) {
                lenToAllocate = sizeof(char) * atoi(buff);
                buff = realloc(buff, lenToAllocate); //adesso buff può contenere char * len
                read(sock, buff, lenToAllocate); // leggo il campo successivo
                msg_T->receiver = malloc(lenToAllocate);
                msg_T->receiver = strdup(buff);
            } else {
                // altrimenti se appartiene alle altre casistiche,
                // il campo 'receiver' nella struttura deve essere vuoto
                msg_T->receiver = malloc(sizeof(""));
                msg_T->receiver = strdup("");
            }


            // qua leggo len e msg
            // len lo facciamo un po più grande: 5 digit
            buff = realloc(buff, 5); // adesso può contenere 5 decimali
            read(sock, buff, 5); // leggo da sock la lunghezza del prossimo campo
            msg_T->msglen = atoi(buff);


            // essendo il campo msg vuoto, non devo leggerlo.
            // nella struttura sarà riempito con " "
            if (msg_T->type == MSG_LOGOUT || msg_T->type == MSG_LIST) {
                msg_T->msg = malloc(sizeof(""));
                msg_T->msg = strdup("");
            } else {
                lenToAllocate = sizeof(char) * atoi(buff);
                buff = realloc(buff, lenToAllocate); //adesso buff può contenere char * len
                read(sock, buff, lenToAllocate); // leggo il campo successivo
                msg_T->msg = malloc(lenToAllocate);
                msg_T->msg = strdup(buff);

                // questo servirà dopo per il login
                // strtok divide la stringa, quindi ho bisogno di una copia
                tmpBuff = malloc(lenToAllocate);
                tmpBuff = strdup(buff);
            }



            printf("type: %c\n", msg_T->type);
            printf("sender: %s\n", msg_T->sender);
            printf("receiver: %s\n", msg_T->receiver);
            printf("msglen: %d\n", msg_T->msglen);
            printf("mesg: %s\n", msg_T->msg);


            // TODO //
            // In ogni caso il thread worker deve scrivere sul log file
            // il comando che dovrà eseguire
            /*strcat(logMsg, msg_T->type);
            strcat(logMsg, ":");
            strcat(logMsg, msg_T->msg);
            buildLog(logMsg, 0);*/


            if (msg_T->type == MSG_REGLOG) {

                // prima inserisco il nuovo utente nella hash table
                // buff contiene ancora il messaggio ricevuto dal client
                userName = strtok(tmpBuff, ":");
                // pulisco il buffer per poterlo utilizzare per inviare messaggi
                // al client
                bzero(tmpBuff, lenToAllocate);

                if (registerNewUser(buff, user) && loginUser(userName, bs, sock)) {
                    // se sia la registrazione che il login sono avvenuti con successo
                    // costruisco il messaggio di risposta per il client
                    lenToAllocate = sizeof(char);
                    tmpBuff = realloc(tmpBuff, lenToAllocate);
                    sprintf(tmpBuff,"%c", MSG_OK);
                } else {
                    // altrimenti costruisce il messaggio di errore
                    lenToAllocate = sizeof(char) * 61;
                    tmpBuff = realloc(tmpBuff, lenToAllocate);
                    sprintf(tmpBuff,"%c", MSG_ERROR);
                    strcat(tmpBuff, "061");
                    strcat(tmpBuff, "Error during Registration... ( username already taken ? )");
                }

            }
/*            if (msg_T->type == MSG_LIST) {
                // la funzione sottostante ritorna un buffer
                // contenente la lista di utenti online ( sockid != -1 )

            }*/


            // la send sottostante ha il compito di informare il client se
            // le operazioni richieste sono andate a buon fine

            if (msg_T->type == MSG_REGLOG) {
                if(send(sock , tmpBuff , lenToAllocate , 0) < 0) {
                    buildLog("[!] Cannot send Info to the client!", 1);
                }
            }
        }


    }
    //free(buff);
    pthread_exit(NULL);
}

bool registerNewUser(char *msg, hdata_t *user) {

    // variabili necessarie all'inserimento dei dati nella hash table
    char *userName;
    char *fullName;
    char *mail;

    userName = strtok(msg, ":");

    // se il nome utente non è ancora stato usato
    if (CERCAHASH(userName, HASH_TABLE) == NULL) {
        fullName = strtok(NULL, ":");
        mail = strtok(NULL, ":");

        user->uname = userName;
        user->fullname = fullName;
        user->email = mail;
        user->sockid = -1;

        INSERISCIHASH(user->uname, (void*) user, HASH_TABLE);
        return true;
    }
    return false;
}

bool loginUser(char *user, hdata_t *bs, int sock) {

    bs = CERCAHASH(user, HASH_TABLE);
    if ( bs == NULL ) {
        buildLog("Username not Found", 0);
        return false;
    }
    bs->sockid = sock;
    return true;
}


/*char *listUser(hdata_t *online) {

}*/
