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


char *buildMsgForSocket(char *tmpBuff, int success);


void *launchThreadWorker(void *newConn) {

    //pthread_mutex_t mux = PTHREAD_MUTEX_INITIALIZER;


    char *buff = malloc(sizeof(char)); // la dimensione iniziale è quella di un char
                                        // ovvero il primo token da leggere

    char *tmpBuff = malloc(sizeof(char));

    // questa variabile verrà riempita dopo la registrazione con
    // il nome dell'utente che dovrà effettuare il login
    char *userName;

    int lenToAllocate;
    int sock = *(int*)newConn;

    char logMsg[256];

    int success = 0;
    int forCounter;

    bool go = true;

    msg_t *msg_T = malloc(sizeof(struct msg_t*));

    hdata_t *hashUser = (hdata_t *) malloc(sizeof(struct msg_t*));

    // 'pulisco' il buffer
    // 'pulisco' l'array dei log
    bzero(buff, sizeof(char));
    bzero(logMsg, 256);


    // Viene ricevuto il messaggio e controllato quale servizio
    // viene richiesto

    while(go && (read(sock, buff, sizeof(char)) > 0)) {


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
        for (forCounter = 0; forCounter < 2; forCounter++) {

            buff = realloc(buff, 3); // adesso può contenere 3 decimali
            read(sock, buff, 3); // leggo da sock la lunghezza del prossimo campo

            // ANNOTAZIONE
            // il sender non è mai presente
            if (atoi(buff) != 0) {
                lenToAllocate = sizeof(char) * atoi(buff);
                buff = realloc(buff, lenToAllocate); //adesso buff può contenere char * len
                read(sock, buff, lenToAllocate); // leggo il campo successivo
                if (forCounter == 0) {
                    msg_T->sender = malloc(lenToAllocate);
                    msg_T->sender = strdup(buff);
                } else {
                    msg_T->receiver = malloc(lenToAllocate);
                    msg_T->receiver = strdup(buff);
                }
            }

        }


        // qua leggo len e msg
        // len lo facciamo un po più grande: 5 digit
        buff = realloc(buff, 5); // adesso può contenere 5 decimali
        read(sock, buff, 5); // leggo da sock la lunghezza del prossimo campo
        msg_T->msglen = atoi(buff);


        if(atoi(buff) != 0) {
            lenToAllocate = sizeof(char) * atoi(buff);
            buff = realloc(buff, lenToAllocate); //adesso buff può contenere char * len
            read(sock, buff, lenToAllocate); // leggo il campo successivo
            msg_T->msg = malloc(lenToAllocate);
            msg_T->msg = strdup(buff);
        }

        if (msg_T->type == MSG_REGLOG || msg_T->type == MSG_LOGIN) {
            // questo servirà dopo per il login
            // strtok divide la stringa, quindi ho bisogno di una copia
            tmpBuff = malloc(lenToAllocate);
            tmpBuff = strdup(buff);
        }
        // ORA TUTTO IL MESSAGGIO È STATO MESSO DENTRO LA STRUTTURA

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

        userName = strtok(msg_T->msg, ":");
        // pulisco il buffer per poterlo utilizzare per inviare messaggi
        // al client
        bzero(tmpBuff, lenToAllocate);


        // leggere i commenti di 'buildMsgForSocket()''
        if (msg_T->type == MSG_REGLOG &&
            !(registerNewUser(buff, hashUser) &&
                loginUser(userName, hashUser, sock))) {

            success = -1;
        } else if (msg_T->type == MSG_LOGIN &&
            !(loginUser(userName, hashUser, sock))) {

            success = -2;
        } else if (msg_T->type == MSG_LOGOUT) {

            go = false;
        }

            /*if (msg_T->type == MSG_LIST) {
                // la funzione sottostante ritorna un buffer
                // contenente la lista di utenti online ( sockid != -1 )

            }*/

        buildMsgForSocket(tmpBuff, success);

        // la send sottostante ha il compito di informare il client se
        // le operazioni richieste sono andate a buon fine o meno
        if(send(sock , tmpBuff , strlen(tmpBuff) , 0) < 0) {
            buildLog("[!] Cannot send Info to the client!", 1);
        }

    }
    close(sock);
    pthread_exit(NULL);
}

char *buildMsgForSocket(char *tmpBuff, int success) {

    /* la gestione del successo o meno delle azioni richieste dipende da questa funzione.
    Se l'azione è stata eseguita con successo, la variabile 'success' ha valore 0
    quindi il messaggio da spedire al client è MSG_OK.
    In caso contrario 'success' avrà un valore negativo. In base alla tabella
    sottostante vengono creati diversi messaggi di errore da spedire al client*/

    /*
    success == -1 -> registrazione || login falliti ( username già presente nella table )
    success == -2 -> login fallito
    */

    if (success == 0) {
        sprintf(tmpBuff,"%c", MSG_OK);
    } else {

        // in caso di 'success' < 0 il messaggio ritornato dalla funzione è composto come
        // E<lunghezza del messaggio><testo del messaggio>
        if (success == -1) {
            tmpBuff = realloc(tmpBuff, sizeof(char) * 61);
            sprintf(tmpBuff,"%c057Error during Registration... ( username already taken ? )", MSG_ERROR);
        }
        if (success == -2) {
            tmpBuff = realloc(tmpBuff, sizeof(char) * 44);
            sprintf(tmpBuff,"%c040Error during Login... Username not found", MSG_ERROR);
        }
    }
    return tmpBuff;

}
