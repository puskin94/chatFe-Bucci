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

/* la funzione sottostante consente di caricare tutti i dati necessari
alla gestione degli utenti nella tabella hash */

bool readUserFile() {

    FILE *fp;
    char userInfo[771]; // 768 == (256 * 3) + 2 + 1 --> (single max lenght + 2 (:) + 1 (\0))

    char *userName;
    char *fullName;
    char *mail;

    hdata_t *user;

    fp = fopen(userFile, "r+");
    if (fp != NULL) {

        HASH_TABLE = CREAHASH();

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

    userName = strdup(strtok(msg, ":"));

    // se il nome utente non è ancora stato usato
    if (CERCAHASH(userName, HASH_TABLE) == NULL) {
        fullName = strdup(strtok(NULL, ":"));
        mail = strdup(strtok(NULL, ":"));

        user->uname = userName;
        user->fullname = fullName;
        user->email = mail;
        user->sockid = -1;

        INSERISCIHASH(user->uname, (void*) user, HASH_TABLE);
        return true;
    }
    return false;
}

int loginUser(char *user, hdata_t *bs, int sock) {

    // se l'username è presente e non è ancora loggato, ritorna 0
    // se l'username non è presente, ritorna -2
    // se l'utente è presente ma è già loggato, ritorna -3
    char msg[100];
    bs = CERCAHASH(user, HASH_TABLE);
    if (bs == NULL) {
        buildLog("Username not Found", 0);
        printf("ritorno -2\n");
        return -2;
    } else if (bs->sockid == -1) {
        bs->sockid = sock;
        strcpy(msg, user);
        strcat(msg, " has logged");
        buildLog(msg, 0);
        return 0;
    }
    return -3;
}


// questa funzione non funziona

char *listUser(char *buff, hdata_t *online) {
    printf("lsakjdlkasjd\n");

    lista L;
    char *test;
    L = (lista) malloc(sizeof(struct cella));
    printf("lsakjdlkasjd\n");
    L = PRIMOLISTA(L);
    printf("lsakjdlkasjd\n");

    // se la lista non è vuota
    if (LISTAVUOTA(L) == 1) {
        printf("lulz\n");
        while(SUCCLISTA(L) != NULL) {
            test = strdup(L->elemento);
            printf("--->%s\n", test);
            L = SUCCLISTA(L);
        }
    } else {
        printf("lelz\n");
    }

/*    tmpBuff = realloc(tmpBuff, sizeof(char) * 61);
    sprintf(tmpBuff,"%c057Error during Registration... ( username already taken ? )", MSG_LIST);
*/
    return "leeel";
}
