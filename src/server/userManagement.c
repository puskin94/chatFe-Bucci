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
#include "include/threadWorker.h"
#include "include/lista.h"


static hash_t HASH_TABLE;
static lista OnUser;
static lista NewUser;
static hdata_t *bs;


static pthread_mutex_t userMux = PTHREAD_MUTEX_INITIALIZER;

bool isInTable(char *user) {
    return (CERCAHASH(user, HASH_TABLE) == NULL) ? false : true;
}

int returnSockId(char *user, hdata_t *bs) {
    pthread_mutex_lock(&userMux);
    bs = CERCAHASH(user, HASH_TABLE);
    pthread_mutex_unlock(&userMux);
    return bs->sockid;
}

/* la funzione sottostante consente di caricare tutti i dati necessari
alla gestione degli utenti nella tabella hash */

bool readUserFile() {

    FILE *fp;

    char userInfo[771];
    char *userName, *fullName, *mail;

    hdata_t *user;

    fp = fopen(userFile, "r+");
    if (fp == NULL) {
        fp = fopen(userFile, "w");
        readUserFile();
    } else {

        HASH_TABLE = CREAHASH();
        OnUser = CREALISTA();
        NewUser = CREALISTA();

        while (fgets (userInfo, sizeof(userInfo), fp)) {

            user = (hdata_t *) malloc(sizeof(hdata_t));

            userName = strdup(strtok(userInfo, ":"));
            fullName = strdup(strtok(NULL, ":"));
            mail = strdup(strtok(NULL, ":\n"));

            if (userName != NULL && fullName != NULL && mail != NULL) {

                user->uname = userName;
                user->fullname = fullName;
                user->email = mail;
                user->sockid = -1;

                INSERISCIHASH(user->uname, (void*) user, HASH_TABLE);
            }
        }

        free(userName); free(fullName); free(mail);
    }
    fclose(fp);
    return true;

}

bool registerNewUser(char *msg) {

    // variabili necessarie all'inserimento dei dati nella hash table
    char *userName, *fullName, *mail;

    bs = (hdata_t *) malloc(sizeof(struct msg_t*));

    userName = strdup(strtok(msg, ":"));

    pthread_mutex_lock(&userMux);
    // se il nome utente non è ancora stato usato
    if (isInTable(userName)) {
        pthread_mutex_unlock(&userMux);
        return false;
    }
    fullName = strdup(strtok(NULL, ":"));
    mail = strdup(strtok(NULL, ":"));

    bs->uname = userName;
    bs->fullname = fullName;
    bs->email = mail;
    bs->sockid = -1;

    INSERISCIHASH(bs->uname, (void*)bs, HASH_TABLE);

        /* inserisco il nuovo utente in una lista apposita che contiene
        lo username di tutti gli utenti che si sono registrati dall'ultima
        accensione del server */
    posizione lastElem = ULTIMOLISTA(NewUser);
    INSLISTA(userName, &lastElem);

    pthread_mutex_unlock(&userMux);
    return true;
}

int loginUser(char *user, int sock) {

    // se l'username è presente e non è ancora loggato, ritorna 0
    // se l'username non è presente, ritorna -2
    // se l'utente è presente ma è già loggato, ritorna -3
    char *msg;

    bs = (hdata_t *) malloc(sizeof(struct msg_t*));

    pthread_mutex_lock(&userMux);
    bs = CERCAHASH(user, HASH_TABLE);
    posizione lastElem = ULTIMOLISTA(OnUser);
    if (bs == NULL) {
        buildLog("Username not Found", 0);
        pthread_mutex_unlock(&userMux);
        return -2;
    } else if (bs->sockid == -1) {
        // modifico il sockid nella hashtable
        bs->sockid = sock;
        // ed inserisco l'utente nella lista
        INSLISTA(user, &lastElem);

        msg = malloc(strlen(user) + 15);
        sprintf(msg, "%s has logged in", user);
        buildLog(msg, 0);
        free(msg);
        pthread_mutex_unlock(&userMux);
        return 0;
    }
    pthread_mutex_unlock(&userMux);
    return -3;
}

void logout(char *user) {

    char *msg;
    bool deleted = false;

    bs = (hdata_t *) malloc(sizeof(struct msg_t*));

    pthread_mutex_lock(&userMux);
    posizione el = PRIMOLISTA(OnUser);

    // prima cambio il suo sockid nella Hash_Table
    bs = CERCAHASH(user, HASH_TABLE);
    bs->sockid = -1;
    // poi rimuovo il suo nome dalla lista degli OnUser

    while (PREDLISTA(el) != ULTIMOLISTA(OnUser) && !deleted) {
        if (strcmp(el->elemento, user) == 0) {
            CANCLISTA(&el);
            deleted = true;
        }
        el = SUCCLISTA(el);
    }
    // scrivo nel log-file che l'utente si è disconnesso
    msg = malloc(strlen(user) + 16);
    sprintf(msg, "%s has logged out", user);
    buildLog(msg, 0);
    pthread_mutex_unlock(&userMux);
    free(msg);
}

void listUser(char **tmpBuff) {
    bool isFirst = true;
    char *buff;

    /* la funzione modifica un buffer che gli viene passato come argomento.
    Questo conterrà la lista degli utenti connessi separati da il simbolo ":".
    La lista sarà preceduta da 6 digit che serviranno al client per leggere
    il messaggio una volta sola e di elaborarlo in locale.*/
    pthread_mutex_lock(&userMux);
    posizione el = PRIMOLISTA(OnUser);

    while (PREDLISTA(el) != ULTIMOLISTA(OnUser)) {
        if (!isFirst) {
            buff = realloc(buff, (strlen(buff) + strlen(el->elemento) + 1) * sizeof(char));
            strcat(buff, ":");
            strcat(buff, el->elemento);
        } else {
            buff = malloc(sizeof(el->elemento) * sizeof(char));
            bzero(buff, strlen(el->elemento) * sizeof(char));
            strcat(buff, el->elemento);
            isFirst = false;
        }
        el = SUCCLISTA(el);
    }
    *tmpBuff = realloc(*tmpBuff, 7 + sizeof(buff));
    bzero(*tmpBuff, 7 + sizeof(buff));
    sprintf(*tmpBuff, "%06zu%s", strlen(buff), buff);

    pthread_mutex_unlock(&userMux);
}

void saveTable() {

    FILE *fp;

    /* viene aperto il file in modalità 'append'. Il file quindi non viene
    sovrascritto ma viene invece 'aggiornato' con nuove informazioni */
    fp = fopen(userFile, "a");
    if (fp != NULL) {

        posizione elem = PRIMOLISTA(NewUser);
        hdata_t *bs = NULL;

        // scorro tutta la lista dei nuovi utenti
        while (PREDLISTA(elem) != ULTIMOLISTA(NewUser)) {
            if ((bs = CERCAHASH(elem->elemento, HASH_TABLE)) != NULL) {
                // stampo i dati di ogni utente nel file
                fprintf(fp, "%s:%s:%s\n", bs->uname,
                                            bs->fullname,
                                            bs->email);

                elem = SUCCLISTA(elem);
            }
        }

    }
    fclose(fp);
}
