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

void *launchThreadWorker(int newConn) {

    char buf[100];

    if(read(newConn, buf, sizeof(buf)) < 0) {
        printf("Received nothing\n");
    }
    printf("%s\n", buf);
    return NULL;
}
