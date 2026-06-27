#ifndef H_STRUCT_H
#include "struct.h"
#endif

//compteur.c
void compteur_init();
void compteur_refresh( int *compteur, char *etat );
void main_loop( int *compteur, char *etat);
void compteur_sleep(int msec);
int ncurse_get_key ();
void clear_term();
