#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //pour sleep, a retirer apres debugage
#include <string.h> //utile pour memcpy pour copier la struction PosPiece
#include <locale.h>
#include <ncursesw/ncurses.h>
#include <time.h>
#include <pulse/simple.h> //pour le son bip
#include <pulse/error.h> //pour le son bip
#include <math.h> //nécessaire pour créer le son sinusoidal

#include "headers/fnsdefs.h"
#include "headers/struct.h"

#define SAMPLE_RATE 44100

WINDOW *CompteurWin;

int main( int argc, char **argv){
	int *compteur ;
	char *etat;
	compteur = malloc (sizeof (int));
	etat = malloc (sizeof (char));
	*etat ='r';
	setlocale(LC_ALL, "");
	if (argc <2 || argc> 3){
	}
	if (argc == 2){
		*compteur = atoi(argv[1]);
	}else if (argc ==3){
		*compteur = atoi(argv[1])*60;
		*compteur += atoi(argv[2]);
	}else{
		printf("command: %s time-to-spend-in-second\n", argv[0]);
		printf("or command: %s time-to-spend-in-minutes and-seconds\n", argv[0]);
		printf("example: %s 60\n", argv[0]);
		printf("for 60s countdown.\n");
		printf("example: %s 5 30\n", argv[0]);
		printf("for 5m30 countdown.\n");
	
		exit(1);
	}
	initscr();              // Initialise la structure WINDOW et autres paramètres
	cbreak();
	keypad(stdscr, TRUE);	// permet de lire les entrées claviers spéciales en une fois( et non trois caractères pour les touches des flèches)
	noecho();
	keypad(stdscr, TRUE); // permet de capter le signal KEY_RESIZE dans les caractères tapés pour faire le redimensionnement du terminal
	
	compteur_init();


	main_loop (compteur, etat);

//remet bien ncurses
	clear_term ();
}



void compteur_init(){
	CompteurWin = subwin(stdscr, 7 , 30 , 2, 5);
	box(CompteurWin, ACS_VLINE, ACS_HLINE);
	wrefresh(CompteurWin);
	wnoutrefresh(CompteurWin); //sert à effacer les données de fenetre en tampon
}

void compteur_refresh( int *compteur, char *etat ){
	int minutes;
	int secondes;

	werase(CompteurWin); //sert à effacer les données de fenetre en tampon
	mvwin( CompteurWin , 2, 5);
	wnoutrefresh(CompteurWin); //sert à effacer les données de fenetre en tampon
	wresize( CompteurWin , 7 , 30 );
	wnoutrefresh(CompteurWin); //sert à effacer les données de fenetre en tampon

	minutes = *compteur / 60	;
	secondes = *compteur - ( minutes * 60);
	
	mvwprintw( CompteurWin , 2 ,2 , "time left %d m %d %s", minutes, secondes, (*etat == 'p')?"- pause":"");
	mvwprintw( CompteurWin , 3 ,2 , "press q to quit!");
	mvwprintw( CompteurWin , 4 ,2 , "press p to pause!");

	box(CompteurWin, ACS_VLINE, ACS_HLINE);
	wrefresh(CompteurWin);
	wnoutrefresh(CompteurWin); //sert à effacer les données de fenetre en tampon

	wrefresh(CompteurWin);
	refresh();
}

void main_loop( int *compteur, char *etat ){
	int error;
	pa_simple *s;
	compteur_refresh( compteur, etat );
	while (*etat != 'q'){
		switch (*etat){
			//running
			case 'r':
				switch (ncurse_get_key()){
					case 'p':
						*etat='p';
						continue;
					case 'q':
						*etat='q';
						continue;
				}
				(*compteur)--;
				if (*compteur == 0){
					(*etat) = 's';
				}
				compteur_sleep (1000);
				compteur_refresh (compteur, etat);
				continue;
			//pause
			case 'p':
				switch (ncurse_get_key()){
					case 'p':
						*etat='r';
						continue;
					case 'q':
						*etat='q';
						continue;
				}
				
				compteur_sleep (10);
				compteur_refresh (compteur, etat);
				continue;
			//sonnerie
			case 's':
				compteur_refresh (compteur, etat);

				static const pa_sample_spec ss = {
					.format = PA_SAMPLE_S16LE,
					.rate = SAMPLE_RATE,
					.channels = 1
				};
				s = pa_simple_new(
					NULL,                // serveur par défaut
					"Compte-minute",        // nom de l'app
					PA_STREAM_PLAYBACK,
					NULL,               // périphérique par défaut
					"tone",             // description
					&ss,
					NULL,
					NULL,
					&error
				);

				if (!s) {
					fprintf(stderr, "Erreur PulseAudio: %s\n", pa_strerror(error));
					exit(1);
				}
				int duration = 2; // secondes
				for (int i = 0; i < SAMPLE_RATE * duration; i++) {
					double t = (double)i / SAMPLE_RATE;
					short sample = (short)(sin(2 * M_PI * 440 * t) * 30000* sin(2 * M_PI * 5 * t));

					if (pa_simple_write(s, &sample, sizeof(sample), &error) < 0) {
					fprintf(stderr, "Erreur write: %s\n", pa_strerror(error));
					exit( 1);
					}
				}
				pa_simple_drain(s, &error);
				pa_simple_free(s);
				*etat = 'q';
				continue;
		}
	}
}

void compteur_sleep(int msec){
	struct timespec ts;
	ts.tv_sec = msec / 1000;
	ts.tv_nsec = (msec - ts.tv_sec *1000) * 1000 * 1000; // 1 ms = 1 000 000 ns

	nanosleep(&ts, NULL);
}


int ncurse_get_key (){

	int ch;

	noecho();
	cbreak();

	nodelay(stdscr, TRUE);   // getch() devient non-bloquant
	
	ch = getch();
	if (ch != ERR) {
		return ch;
	}
	return 0;
}

void clear_term (){
	delwin(CompteurWin);
	endwin();               // Restaure les paramètres par défaut du terminal
}
