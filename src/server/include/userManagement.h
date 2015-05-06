#ifndef USERMANAGEMENT_H
#define USERMANAGEMENT_H

#include <stdbool.h>

#include "common.h"
#include "lista.h"

bool readUserFile();
bool registerNewUser(char *msg, hdata_t *user);
bool loginUser(char *user, hdata_t *bs, int sock);
char *listUser(hdata_t *online);

#endif
