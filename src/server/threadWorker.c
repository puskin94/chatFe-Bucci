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


char *buildMsgForSocket(int success);
void readAndLoadFromSocket(msg_t *msg_T,int sock, char type);


void *launchThreadWorker(void *newConn) {

    //pthread_mutex_t mux = PTHREAD_MUTEX_INITIALIZER;

    char *buff = malloc(sizeof(char)); // la dimensione iniziale è quella di un char
                                        // ovvero il primo token da leggere
    char *tmpBuff;

    // questa variabile verrà riempita dopo la registrazione con
    // il nome dell'utente che dovrà effettuare il login
    char *userName;

    int sock = *(int*)newConn;

    char logMsg[256];

    int success = 0;

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

        readAndLoadFromSocket(msg_T, sock, buff[0]);

        // TODO //
        // In ogni caso il thread worker deve scrivere sul log file
        // il comando che dovrà eseguire
        /*strcat(logMsg, msg_T->type);
        strcat(logMsg, ":");
        strcat(logMsg, msg_T->msg);
        buildLog(logMsg, 0);*/

        userName = strdup(msg_T->msg);
        userName = strtok(userName, ":");

        // leggere i commenti di 'buildMsgForSocket()''
        if (msg_T->type == MSG_REGLOG &&
            !(registerNewUser(msg_T->msg, hashUser) &&
                loginUser(userName, hashUser, sock) == 0)) {

            success = -1;
        } else if (msg_T->type == MSG_LOGIN) {

            success = loginUser(userName, hashUser, sock);
        }

        tmpBuff = buildMsgForSocket(success);

        // la send sottostante ha il compito di informare il client se
        // le operazioni richieste sono andate a buon fine o meno
        if(send(sock , tmpBuff , strlen(tmpBuff) , 0) < 0) {
            buildLog("[!] Cannot send Info to the client!", 1);
        }


        tmpBuff = realloc(tmpBuff, sizeof(char));
        // In questo ciclo inizia il while che consente lo scambio interattivo
        // di messaggi tra client e server. Viene eseguito solamente se il server
        // ha risposto affermativamente al comando iniziale inviato dal client
        while (success == 0 && (read(sock, tmpBuff, sizeof(char)) > 0)) {
            readAndLoadFromSocket(msg_T, sock, buff[0]);
        }


    }
    close(sock);
    pthread_exit(NULL);
}

char *buildMsgForSocket(int success) {

    /* la gestione del successo o meno delle azioni richieste dipende da questa funzione.
    Se l'azione è stata eseguita con successo, la variabile 'success' ha valore 0
    quindi il messaggio da spedire al client è MSG_OK.
    In caso contrario 'success' avrà un valore negativo. In base alla tabella
    sottostante vengono creati diversi messaggi di errore da spedire al client*/

    /*
    success == -1 -> registrazione || login falliti ( username già presente nella table )
    success == -2 -> login fallito
    */

    char *tmpBuff = malloc(sizeof(char));

    switch(success) {
        case 0:
            sprintf(tmpBuff,"%c", MSG_OK);
            break;
        case -1:
            tmpBuff = realloc(tmpBuff, sizeof(char) * 61);
            sprintf(tmpBuff,"%c057Error during Registration... ( username already taken ? )", MSG_ERROR);
            break;
        case -2:
            tmpBuff = realloc(tmpBuff, sizeof(char) * 44);
            sprintf(tmpBuff,"%c040Error during Login... Username not found", MSG_ERROR);
            break;
        case -3:
            tmpBuff = realloc(tmpBuff, sizeof(char) * 27);
            sprintf(tmpBuff, "%c023You are already Logged!", MSG_ERROR);
            break;
    }

    return tmpBuff;

}

void readAndLoadFromSocket(msg_t *msg_T, int sock, char type) {

    int lenToAllocate;
    char *buffer = malloc(sizeof(char)); // la dimensione iniziale è quella di un char
                                        // ovvero il primo token da leggere
    int forCounter;


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


    msg_T->type = type;


    for (forCounter = 0; forCounter < 2; forCounter++) {

        buffer = realloc(buffer, 3); // adesso può contenere 3 decimali
        read(sock, buffer, 3); // leggo da sock la lunghezza del prossimo campo

            // ANNOTAZIONE
            // il sender non è mai presente
        if (atoi(buffer) != 0) {
            lenToAllocate = sizeof(char) * atoi(buffer);
            buffer = realloc(buffer, lenToAllocate); //adesso buff può contenere char * len
            read(sock, buffer, lenToAllocate); // leggo il campo successivo
            if (forCounter == 0) {
                msg_T->sender = malloc(lenToAllocate);
                msg_T->sender = strdup(buffer);
            } else {
                msg_T->receiver = malloc(lenToAllocate);
                msg_T->receiver = strdup(buffer);
            }
        }

    }

        // qua leggo len e msg
        // len lo facciamo un po più grande: 5 digit
    buffer = realloc(buffer, 5); // adesso può contenere 5 decimali
    read(sock, buffer, 5); // leggo da sock la lunghezza del prossimo campo
    msg_T->msglen = atoi(buffer);


    if(msg_T->msglen != 0) {
        lenToAllocate = sizeof(char) * msg_T->msglen;
        buffer = realloc(buffer, lenToAllocate); //adesso buff può contenere char * len
        read(sock, buffer, lenToAllocate); // leggo il campo successivo
        msg_T->msg = malloc(lenToAllocate);
        msg_T->msg = strdup(buffer);
    }
        // ORA TUTTO IL MESSAGGIO È STATO MESSO DENTRO LA STRUTTURA

    printf("type: %c\n", msg_T->type);
    printf("sender: %s\n", msg_T->sender);
    printf("receiver: %s\n", msg_T->receiver);
    printf("msglen: %d\n", msg_T->msglen);
    printf("mesg: %s\n", msg_T->msg);
}
