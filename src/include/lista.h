/*************************************/
/* Library list: schifano 2013       */
/*************************************/

#ifndef LISTA_H
#define LISTA_H

#include <stdio.h>
#include <stdlib.h>

typedef struct cella * lista;
typedef lista posizione;

struct cella {
  posizione precedente;
  void *    elemento;
  posizione successivo;
};


lista CREALISTA () ;

int LISTAVUOTA (lista L);


posizione PRIMOLISTA (lista L);


posizione ULTIMOLISTA (lista L);


posizione SUCCLISTA (posizione p);


posizione PREDLISTA (posizione p) ;


int FINELISTA (posizione p, lista L);


void INSLISTA (void * data, posizione * p);


void CANCLISTA (posizione * p) ;

#endif
