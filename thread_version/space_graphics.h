/** Space Invaders: 8 bit attack!
 *
 * File: space_graphics.h
 *
 * Contiene: dichiarazioni e istruzioni di preprocessing relative
 * 		al file space_graphics.c
 */

#ifndef SPACE_GRAPHICS_H
#define SPACE_GRAPHICS_H

#include <curses.h>

/**Nomi mnemonici per selezionare le astronavi**/
#define ENTERPRISE 0
#define BOX 1

/**Dimensione delle astronavi**/
#define SSX 6	//Astronave giocatore (def. 6x6)
#define SSY 6
#define E1X 3	//Nemico di livello 1 (def. 3x3)
#define E1Y 3
#define E2X 4	//Nemico di livello 2 (def. 4x4)
#define E2Y 4
#define E3X 5	//Nemico di livello 3 (def. 5x5)
#define E3Y 5

/**Caratteri speciali aggiuntivi**/
#define SLSH '/'
#define BCKSLSH '\\'

#include "space_game.h"

typedef struct{
	int x;
	int y;
} Coords;

/**Dichiarazione di funzioni**/
void moveShip(int, int, int, int);
void printEntity(int dimy, int dimx, char[dimy][dimx], int, int, int);
chtype chloader(char);
void moveWeapon(int, int, int, int);
void moveEntity(int, int, int, int);
Coords placeEnemies(int);

//Astronavi default
char modelSpaceship[SSY][SSX];
char modelEnemyOne[E1Y][E1X];
char modelEnemyTwo[E2Y][E2X];
char modelEnemyThr[E3Y][E3X];

//Astronavi alternative
char modelBox[SSY][SSX];

//Missile
char modelMissile;

//Bomba
char modelBomb;

#endif
