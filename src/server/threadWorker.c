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
#include "include/threadMain.h"
#include "include/userManagement.h"
#include "include/threadDispatcher.h"
#include "include/threadWorker.h"

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

msg_t structMsg_t;
msg_t *msg_T = &structMsg_t;


void *launchThreadWorker(void *newConn) {

    int success = 0;
    int sock = *(int*)newConn;

    bool go = true;
    bool isLogout = false;

    pthread_mutex_t mux = PTHREAD_MUTEX_INITIALIZER;


    // la variabile 'userName' verrà riempita dopo la registrazione con
    // il nome dell'utente che dovrà effettuare il login
    char *buff, *tmpBuff, *userName, *errorMsg;

    buff = malloc(sizeof(char));
    tmpBuff = malloc(sizeof(char));


    // Viene ricevuto il messaggio e controllato quale servizio
    // viene richiesto

    while(go && !isLogout && (read(sock, buff, sizeof(char) * 6) > 0)) {

        readAndLoadFromSocket(sock, atoi(buff));

        userName = strdup(msg_T->msg);
        userName = strtok(userName, ":");

        // leggere i commenti di 'buildMsgForSocket()''
        if (msg_T->type == MSG_REGLOG &&
            !(registerNewUser(msg_T->msg) &&
                loginUser(userName, sock) == 0)) {

            success = -1;
        } else if (msg_T->type == MSG_LOGIN) {

            success = loginUser(userName, sock);
        }

        buildMsgForSocket(success, &tmpBuff);

        // la send sottostante ha il compito di informare il client se
        // le operazioni richieste sono andate a buon fine o meno
        if(send(sock , tmpBuff , strlen(tmpBuff) , 0) < 0) {
            errorMsg = strdup("[!] Cannot send Infos to the client!");
            buildLog(errorMsg, 1);
        }

        //tmpBuff = realloc(tmpBuff, sizeof(char));
        buff = realloc(buff, sizeof(char) * 6);
        bzero(buff, sizeof(char) * 6);
        //bzero(tmpBuff, sizeof(char));
        // In questo ciclo inizia il while che consente lo scambio interattivo
        // di messaggi tra client e server. Viene eseguito solamente se il server
        // ha risposto affermativamente al comando iniziale inviato dal client
        while (success == 0 && (read(sock, buff, sizeof(char) * 6) > 0)) {

            readAndLoadFromSocket(sock, atoi(buff));

            if (msg_T->type == MSG_LIST) {

                listUser(&tmpBuff);

                if(send(sock , tmpBuff , strlen(tmpBuff) , 0) < 0) {
                    errorMsg = strdup("[!] Cannot send Infos to the client!");
                    buildLog(errorMsg, 1);
                }

            } else if (msg_T->type == MSG_BRDCAST || msg_T->type == MSG_SINGLE) {
                // tmpBuff conterrà il messaggio da spedire al threadDispatcher
                pthread_mutex_lock(&mux);
                msgForDispatcher(userName, &tmpBuff);

                if (tmpBuff[0] != MSG_ERROR) {
                    // viene copiato il messaggio dentro bufferPC
                    writeOnBufferPC(tmpBuff);
                }
                pthread_mutex_unlock(&mux);

            } else if (msg_T->type == MSG_LOGOUT) {
                logout(userName);
                buildMsgForSocket(1, &tmpBuff);
                // avverto il client che la disconnessione è avvenuta con successo
                if(send(sock , tmpBuff , strlen(tmpBuff) , 0) < 0) {
                    errorMsg = strdup("[!] Cannot send Infos to the client!");
                    buildLog(errorMsg, 1);
                }
                isLogout = true;
            }
        }
    }

    free(tmpBuff); free(buff);
    close(sock);
    pthread_exit(NULL);
}

void msgForDispatcher(char *sender, char **tmpBuff) {

    *tmpBuff = malloc(sizeof(char));


    // crea il messaggio solamente se il destinatario è registrato
    if ((msg_T->type == MSG_SINGLE)) {
        if (isInTable(msg_T->receiver)) {
            /* il buffer di risposta deve essere abbastanza grande per contenere il messaggio.
            esso sarà formattato come

            <lunghezza di tutto il messaggio><tipo><lunghezza del receiver>
            <receiver><lunghezza del messaggio><messaggio>
            */
            *tmpBuff = realloc(*tmpBuff, (13 + strlen(msg_T->receiver) + strlen(sender) + msg_T->msglen) * sizeof(char));
            sprintf(*tmpBuff, "%c%03zu%s%03zu%s%05d%s", msg_T->type,
                                                        strlen(msg_T->receiver),
                                                        msg_T->receiver,
                                                        strlen(sender),
                                                        sender,
                                                        msg_T->msglen,
                                                        msg_T->msg);
        } else {
            *tmpBuff = realloc(*tmpBuff, sizeof(char) * 2);
            sprintf(*tmpBuff, "%c", MSG_ERROR);
        }
    } else {
        // se il messaggio è di tipo MSG_BRDCAST
        *tmpBuff = realloc(*tmpBuff, (10 + msg_T->msglen + strlen(sender)) * sizeof(char));
        sprintf(*tmpBuff, "%c%03zu%s%05d%s", msg_T->type,
                                            strlen(sender),
                                            sender,
                                            msg_T->msglen,
                                            msg_T->msg);
    }

}
void buildMsgForSocket(int success, char **tmpBuff) {

    /* la gestione del successo o meno delle azioni richieste dipende da questa funzione.
    Se l'azione è stata eseguita con successo, la variabile 'success' ha valore 0
    quindi il messaggio da spedire al client è MSG_OK.
    La funzione restituisce un messaggio speciale se success == 1
    In caso contrario 'success' avrà un valore negativo. In base alla tabella
    sottostante vengono creati diversi messaggi di errore da spedire al client*/

    /*
    success == 1  -> logout avvenuto con successo
    success == -1 -> registrazione || login falliti ( username già presente nella table )
    success == -2 -> login fallito ( utente non registrato )
    success == -3 -> login fallito ( utente già loggato )
    */

    *tmpBuff = malloc(sizeof(char) * 2);

    switch(success) {
        case 0:
            sprintf(*tmpBuff,"%c", MSG_OK);
            break;
        case 1:
            *tmpBuff = realloc(*tmpBuff, sizeof(char) * 7);
            sprintf(*tmpBuff,"00000%c", MSG_OK);
            break;
        case -1:
            *tmpBuff = realloc(*tmpBuff, sizeof(char) * 62);
            sprintf(*tmpBuff,"%c057Error during Registration... ( username already taken ? )", MSG_ERROR);
            break;
        case -2:
            *tmpBuff = realloc(*tmpBuff, sizeof(char) * 45);
            sprintf(*tmpBuff,"%c040Error during Login... Username not found", MSG_ERROR);
            break;
        case -3:
            *tmpBuff = realloc(*tmpBuff, sizeof(char) * 28);
            sprintf(*tmpBuff, "%c023You are already Logged!", MSG_ERROR);
            break;
    }

}

void readAndLoadFromSocket(int sock, int len) {

    int lenToAllocate = sizeof(char) * len;
    char *buffer = malloc(lenToAllocate); // la dimensione iniziale è quella di un char
                                        // ovvero il primo token da leggere
    char *tmpBuff = malloc(lenToAllocate);
    char *strLen = malloc(sizeof(char));

    int forCounter, charsRead = 0;


        /* nelle successive linee viene riempita la struttura 'msg_T'.
        Il messaggio che arriva nel socket ha le prime 6 cifre che indicano la
        lunghezza dell'intero messaggio. Leggendo prima questa cifre e
        successivamente l'intero messaggio la socket si svuota.
        Agendo su variabili di tipo char* è possibile suddividere il
        messaggio nei rispettivi campi e riempire la struct.
        Se necessario ( per i campi char * ), la grandezza dei componenti
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

    read(sock, buffer, lenToAllocate);

    strncpy(tmpBuff, buffer, 1);
    charsRead += 1;
    msg_T->type = tmpBuff[0];


    bzero(tmpBuff, 1);

    for (forCounter = 0; forCounter < 2; forCounter++) {

        strLen = realloc(strLen, 3); // adesso può contenere 3 decimali
        strncpy(strLen, buffer + charsRead, 3);
        charsRead += 3;

            // ANNOTAZIONE
            // il sender non è mai presente
        if (atoi(strLen) != 0) {
            lenToAllocate = sizeof(char) * atoi(strLen);
            tmpBuff = realloc(tmpBuff, lenToAllocate); //adesso buff può contenere char * len
            strncpy(tmpBuff, buffer + charsRead, atoi(strLen));
            charsRead += atoi(strLen);

            if (forCounter == 0) {
                msg_T->sender = malloc(lenToAllocate);
                msg_T->sender = strdup(tmpBuff);
            } else {
                msg_T->receiver = malloc(lenToAllocate);
                msg_T->receiver = strdup(tmpBuff);
            }
        }
        bzero(tmpBuff, lenToAllocate);

    }

        // qua leggo len e msg
        // len lo facciamo un po più grande: 5 digit
    tmpBuff = realloc(tmpBuff, 5); // adesso può contenere 5 decimali
    strncpy(tmpBuff, buffer + charsRead, 5);
    charsRead += 5;
    msg_T->msglen = atoi(tmpBuff);
    bzero(tmpBuff, 5);


    if(msg_T->msglen != 0) {
        lenToAllocate = sizeof(char) * msg_T->msglen;
        tmpBuff = realloc(tmpBuff, lenToAllocate); //adesso buff può contenere char * len
        strncpy(tmpBuff, buffer + charsRead, msg_T->msglen);
        msg_T->msg = malloc(lenToAllocate);
        msg_T->msg = strdup(tmpBuff);
    }

    bzero(tmpBuff, lenToAllocate);
            // ORA TUTTO IL MESSAGGIO È STATO MESSO DENTRO LA STRUTTURA

/*        printf("type: %c\n", msg_T->type);
        printf("sender: %s\n", msg_T->sender);
        printf("receiver: %s\n", msg_T->receiver);
        printf("msglen: %d\n", msg_T->msglen);
        printf("mesg: %s\n\n", msg_T->msg);*/

}
