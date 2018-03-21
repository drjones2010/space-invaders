/** Space Invaders: 8 bit attack!
 * 
 * File: space_graphics.c
 * 
 * Contiene: modelli grafici
 */

#include "space_game.h"
#include "space_graphics.h"

/************************/
/**Astronavi di default**/
/************************/

//USS Enterprise NCC 1701 (Macro: ENTERPRISE)
char modelSpaceship[SSY][SSX] = {
   // 123456
   // ||||||
	{"SUUUUB"}, //1
	{"|1701|"}, //2
	{"B____S"}, //3
	{"a^++^s"}, //4
	{"IolroI"}, //5
	{"z9xz9x"}  //6
};

//Borg Interceptor
char modelEnemyOne[E1Y][E1X] = {
   // 123
   // |||
	{"ats"}, //1
	{"I#I"}, //2
	{"zbx"}  //3
};

//Mickey Mouse Pants
char modelEnemyTwo[E2Y][E2X] = {
   // 1234
   // ||||
	{"a99s"}, //1
	{"IooI"}, //2
	{"IasI"}, //3
	{"zxzx"}  //4
};

//Empire Star Destroyer
char modelEnemyThr[E3Y][E3X] = {
   // 12345
   // |||||
	{"a999s"},//5
	{"I###I"},//4
	{"BBvSS"},//3
	{" BvS "},//2
	{"  o  "} //1

};

/*************************/
/**Astronavi alternative**/
/*************************/

//A simple box (Macro: BOX)
char modelBox[SSY][SSX] = {
   // 123456
   // ||||||
	{"a9999s"}, //1
	{"I    I"}, //2
	{"I    I"}, //3
	{"I    I"}, //4
	{"I    I"}, //5
	{"z9999x"}  //6
};

/**************************/
/**Missile dell'astronave**/
/**************************/

char modelMissile = '*';

/************************/
/**Bomba dei nemici    **/
/************************/

char modelBomb = '@';





/*******************************************************************************
Prototipo:
	moveShip(int, int, int, int);

Descrizione:
	Sposta la spaceship sullo schermo.

Parametri:
	int shipModel: macro di scelta dell'astronave giocatore			
	int posy: coordinata y dell'astronave
	int posx: coordinata x dell'astronave
	int mode: assume i valori TRUE o FALSE

Tipo di ritorno: 				Nessuno.
*******************************************************************************/
void moveShip(int shipModel, int posy, int posx, int mode){
	switch(shipModel){
		case ENTERPRISE: printEntity(SSY, SSX, modelSpaceship, posy, posx, mode);
						 break;
		case BOX:		 printEntity(SSY, SSX, modelBox, posy, posx, mode);
						 break;
	}
}
/***********************************************************************
 * Nome: moveEntity
 * Parametri:
 * 		y, x: coordinate dell'entità
 * 		level: livello dell'entità
 * 		del: assume i valori TRUE o FALSE
 * Prende in ingresso le coordinate e il livello dell'entità da spostare 
 * e invoca la funzione printEntity
 * ********************************************************************/
void moveEntity(int y, int x, int level, int del){
		switch(level){
		case STARSHIP_LEVEL:
			printEntity(SSY, SSX, modelSpaceship, y, x, del);
			break;
		case ENEMY_LEVEL1:
			printEntity(E1Y, E1X, modelEnemyOne, y, x, del);
			break;
		case ENEMY_LEVEL2:
			printEntity(E2Y, E2X, modelEnemyTwo, y, x, del);
			break;
		case ENEMY_LEVEL3:
			printEntity(E3Y, E3X, modelEnemyThr, y, x, del);
			break;
	}
}


/*******************************************************************************
Nome Funzione:
	printEntity;

Descrizione:
	Stampa una matrice di caratteri in formato ACS alle coordinate
	(posx,posy) se del è 0, altrimenti cancella inserendo spazi bianchi.

Parametri:
	int dimy: 					Dimensione in altezza;
	int dimx: 					Dimenzione in larghezza;
	char entity[dimy][dimx]:	Matrice dell'astronave;
	int posy:					Posizione in y dell'astronave (top left
								corner);
	int posx:					Posizione in x dell'astronave (top left
								corner).
	int del:					Attiva il flag di cancellazione 

Tipo di ritorno: 				Nessuno.
*******************************************************************************/
void printEntity(int dimy, int dimx, char entity[dimy][dimx], int posy, int posx, int del){
	chtype cht;
	int x, y,		//Coordinate di stampa
	    j, k;		//Cursori della matrice
	
	x=posx;
	y=posy;
	
	if (del == TRUE){ //Disegna
		for(j=0; j<dimy; j++){	
			for(k=0; k<dimx; k++){
				cht = chloader(entity[j][k]);
				pthread_mutex_lock(&mutex_curses);
					mvaddch(y, x, cht);
					refresh();
				pthread_mutex_unlock(&mutex_curses);
				x++;
			}
		x=posx;
		y++;
		}
	} else { //Cancella
		for(j=0; j<dimy; j++){
			for(k=0; k<dimx; k++){
				cht = chloader(entity[j][k]);
				pthread_mutex_lock(&mutex_curses);
					mvaddch(y, x, ' ');
					refresh();
				pthread_mutex_unlock(&mutex_curses);
				x++;
			}
		x=posx;
		y++;
		}
	}
}


/*******************************************************************************
Nome Funzione:
	moveWeapon;

Descrizione:
	Stampa un carattere alle coordinate coordinate (posx,posy) se del è 0,
	altrimenti cancella inserendo spazi bianchi.

Parametri:
	int dimy: 					Dimensione in altezza;
	int dimx: 					Dimenzione in larghezza;
	char entity[dimy][dimx]:	Matrice dell'astronave;
	int posy:					Posizione in y dell'astronave (top left
								corner);
	int posx:					Posizione in x dell'astronave (top left
								corner).
	int del:					Attiva il flag di cancellazione 

Tipo di ritorno: 				Nessuno.
*******************************************************************************/
void moveWeapon(int whoami, int y, int x, int del){
	chtype cht;
	
	switch(whoami){
		case ID_MISSILELEFT: 
			if (del == TRUE){ //Disegna
				cht = chloader(modelMissile);
				pthread_mutex_lock(&mutex_curses);
					mvaddch(y, x, cht);	refresh();
				pthread_mutex_unlock(&mutex_curses);
			} else { //Cancella
				cht = chloader(modelMissile);
				pthread_mutex_lock(&mutex_curses);
					mvaddch(y+1, x+1, ' ');	refresh();
				pthread_mutex_unlock(&mutex_curses);
			}
			break;
		case ID_MISSILERIGHT: 	
			if (del == TRUE){ //Disegna
				cht = chloader(modelMissile);
				pthread_mutex_lock(&mutex_curses);
					mvaddch(y, x, cht);	refresh();
				pthread_mutex_unlock(&mutex_curses);
			} else { //Cancella
				cht = chloader(modelMissile);
				pthread_mutex_lock(&mutex_curses);
					mvaddch(y+1, x-1, ' ');	refresh();
				pthread_mutex_unlock(&mutex_curses);
			}
			break;
		case ID_BOMB: 	
			if (del == TRUE){ //Disegna
				cht = chloader(modelBomb);
				pthread_mutex_lock(&mutex_curses);
					mvaddch(y, x, cht);	refresh();
				pthread_mutex_unlock(&mutex_curses);
			} else { //Cancella
				cht = chloader(modelBomb);
				pthread_mutex_lock(&mutex_curses);
					mvaddch(y-1, x, ' '); refresh();
				pthread_mutex_unlock(&mutex_curses);
			}
			break;
	}

}



/*******************************************************************************
Nome Funzione:
	chloader;

Descrizione:
	Restituisce un carattere di tipo chtype (tabella ACS) dato un normale
	carattere di tipo char.

Parametri:
	char c: 					Carattere char in ingresso;

Tipo di ritorno: 				chtype.
*******************************************************************************/
chtype chloader(char c){
	chtype cht;
	switch (c) {
		case 'a': cht = ACS_ULCORNER; break;
		case 'z': cht = ACS_LLCORNER; break;
		case 's': cht = ACS_URCORNER; break;
		case 'x': cht = ACS_LRCORNER; break;
		case '+': cht = ACS_PLUS; break;
		case '^': cht = ACS_UARROW; break;
		case '9': cht = ACS_HLINE; break;
		case 'I': cht = ACS_VLINE; break;
		case 'o': cht = ACS_BULLET; break;
		case 'S': cht = SLSH; break;
		case 'B': cht = BCKSLSH; break;
		case 'U': cht = ACS_S1; break;
		case 'u': cht = ACS_S3; break;
		case 't': cht = ACS_TTEE; break;
		case 'l': cht = ACS_LTEE; break;
		case 'r': cht = ACS_RTEE; break;
		case 'b': cht = ACS_BTEE; break;

		default: cht = c; break;
	}
	return cht;
}
/***********************************************************************
 * Nome placeEnemies
 * Parametri:
 * 		n: numero dei nemici da inserire
 * Prende in ingresso il numero di nemici e ne restituisce la posizione
 * ********************************************************************/
Coords placeEnemies(int n){	//n: 1->M numero delle astronavi nemiche
	Coords pos;
	

	
	
	pos.x=7*((n-1)%(MAXX/VERTICAL_DISTANCE));
	pos.y=((n-1)/(MAXX/VERTICAL_DISTANCE))*VERTICAL_DISTANCE;	
	
	//~ pos.x = MAXX/2;	//Posiziona al centro dello schermo
	//~ pos.y = MAXY/5; 
	
	return pos;
}
