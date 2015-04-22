#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>

void timestamp(char * ts) {
    time_t t;
    t = time(NULL);
    ctime_r(&t,ts);
    ts[strlen(ts)-1] = '\0';
}

bool createLogFile(char *file) {

    FILE *logFile;

    char ts[64];
    char delimiter[100] = "**************************************************\n";
    char semiDelimiterL[100] = "***  ";
     char semiDelimiterR[100] = " ***";
    char firstMessage[600] = "";
    timestamp(ts);


    strcat(firstMessage, delimiter);
    strcat(firstMessage, semiDelimiterL);
    strcat(firstMessage, "Server Started @ ");
    strcat(firstMessage, ts);
    strcat(firstMessage, semiDelimiterR);
    strcat(firstMessage, "\n");
    strcat(firstMessage, delimiter);

    logFile = fopen(file, "w");
    if (logFile != NULL) {
        if (fprintf(logFile,"%s\n", firstMessage)) {
            return true;
        } else {
            fprintf(stderr,"Cannot write to log-file. Quitting...\n");
            return false;
        }
    } else {
        fprintf(stderr,"Cannot create log-file. Quitting...\n");
        return false;
    }
}

bool readUserFile(char *file) {

    FILE *userFile;
    char *line = NULL;
    char userInfo[771]; // 768 == (256 * 3) + 2 + 1 --> (single max lenght + 2 (:) + 1 (\0))

    char *userName;
    char *fullName;
    char *mail;

    userFile = fopen(file, "r+");
    if (userFile != NULL) {
        while (fgets (userInfo, sizeof(userInfo), userFile)) {
            userName = strtok (userInfo, ":");
            fullName = strtok (NULL, ":");
            mail = strtok (NULL, ":");

            // Qua ci va l'inserimento dei dati nella struttuta ( hash table )

        }

    } else {
        fprintf(stderr,"Error reading userFile\n");
        return false;
    }

    fclose(userFile);
    return true;

}
