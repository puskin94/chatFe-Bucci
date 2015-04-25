#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printHelp() {
    printf("\t\t\tChat-Client help Page\n\n\n");

    printf("Help Page:\n./chatclient -h\n\n");
    printf("Login:\n./chatclient <username>\n\n");
    printf("Register And Login:\n./chatclient -r \"<Name> <Surname> <mail>\" <username>\n\n\n");
    printf("\t\t\tInchat commands\n\n\n");

    printf("Send message to one user:\n#dest <receiver>:text\\n\n\n");
    printf("Send broadcast message:\n#dest :text\\n\n\n");
    printf("List online Users:\n#ls\n\n");
    printf("Logout from Server:\n#logout\n\n");
}
