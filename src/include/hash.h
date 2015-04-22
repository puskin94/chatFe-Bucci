/*************************************/
/* Library hash: schifano fabio 2013 */
/*************************************/

#ifndef HASH_H
#define HASH_H

#include <string.h>

#include "common.h"
#include "lista.h"



typedef lista * hash_t;

//////////////////////////////////////////////////////////////////////

int hashfunc(char * k);

//////////////////////////////////////////////////////////////////////
// restituisce la posizione della cella che contiene la chiave key == uname

hdata_t * CERCALISTA ( char * key, lista L ) ;

//////////////////////////////////////////////////////////////////////

hash_t CREAHASH () ;

//////////////////////////////////////////////////////////////////////
// cerca un elemento nella tabella Hash
// restituisce un "hdata_t *" oppure NULL se non esiste

void * CERCAHASH(char * key, hash_t H);
//////////////////////////////////////////////////////////////////////
// inserisce l'elemento hdata_t * nella tabella hash H

void INSERISCIHASH (char * key, hdata_t * elem, hash_t H);
//////////////////////////////////////////////////////////////////////

#endif
