#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>

#include "include/chat-server.h"
#include "include/utils.h"
#include "include/hash.h"

int launchThreadMain() {
    // read user-file
    if (readUserFile()) {
        printf("[+] SERVER INFO: loaded user-file\n");
    } else {
        printf("[!] SERVER INFO: cannot load user-file\n");
        return -2;
    }

    // write first message to log-file
    if (createLogFile()) {
        printf("[+] SERVER INFO: loaded log-file\n");
    } else {
        printf("[!] SERVER INFO: cannot load log-file\n");
        return -3;
    }
}


/* la funzione sottostante consente di caricare tutti i dati necessari
alla gestione degli utenti nella tabella hash */

bool readUserFile() {

    static hash_t HASH_TABLE;

    FILE *fp;
    char *line = NULL;
    char userInfo[771]; // 768 == (256 * 3) + 2 + 1 --> (single max lenght + 2 (:) + 1 (\0))

    char *userName;
    char *fullName;
    char *mail;

    fp = fopen(userFile, "r+");
    if (fp != NULL) {

        //HASH_TABLE = CREAHASH();
        hdata_t *user = (hdata_t *) malloc(sizeof(hdata_t));

        while (fgets (userInfo, sizeof(userInfo), fp)) {
            userName = strtok (userInfo, ":");
            fullName = strtok (NULL, ":");
            mail = strtok (NULL, ":\n");

            if (userName != NULL && fullName != NULL && mail != NULL) {

                user->uname = userName;
                user->fullname = fullName;
                user->email = mail;
                user->sockid = -1;
            }
        }
    } else {
        buildLog("Cannot load userFile. Quitting...", 1);
        return false;
    }

    fclose(fp);
    return true;

}
