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


static hash_t HASH_TABLE;

/* la funzione sottostante consente di caricare tutti i dati necessari
alla gestione degli utenti nella tabella hash */

bool readUserFile() {

    FILE *fp;
    char userInfo[771]; // 768 == (256 * 3) + 2 + 1 --> (single max lenght + 2 (:) + 1 (\0))

    char *userName;
    char *fullName;
    char *mail;

    fp = fopen(userFile, "r+");
    if (fp != NULL) {

        HASH_TABLE = CREAHASH();
        hdata_t *user = (hdata_t *) malloc(sizeof(hdata_t));

        while (fgets (userInfo, sizeof(userInfo), fp)) {
            userName = strtok(userInfo, ":");
            fullName = strtok(NULL, ":");
            mail = strtok(NULL, ":\n");

            if (userName != NULL && fullName != NULL && mail != NULL) {

                user->uname = userName;
                user->fullname = fullName;
                user->email = mail;
                user->sockid = -1;

                INSERISCIHASH(user->uname, (void*) user, HASH_TABLE);
            }
        }
    } else {
        buildLog("Cannot load userFile. Quitting...", 1);
        return false;
    }

    fclose(fp);
    return true;

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
    if (bs == NULL) {
        buildLog("Username not Found", 0);
        return false;
    }
    bs->sockid = sock;
    return true;
}


/*char *listUser(hdata_t *online) {

}*/
