#ifndef SPACE_MAIN_H
#define SPACE_MAIN_H

/**Varie ed eventuali**/

#define M 5	//Numero dei nemici

#define TRUE 1
#define FALSE 0

#define STARSHIP 1
#define ALL 0

#define WIN 0
#define FAIL 1

#define LEFT -1
#define RIGHT 1
#define DOWN 0

#define MAXX 80
#define MAXY 24
#define READ 0
#define WRITE 1

#define ALLOWED 1
#define DENIED 0

#define FIRE_DELAY_STARSHIP 200000
#define FIRE_DELAY_ENEMY1 9 //in secondi
#define FIRE_DELAY_ENEMY2 7 //in secondi
#define FIRE_DELAY_ENEMY3 5//in secondi

#define MOVE_Y_DELAY 10 //in secondi

#define MISSILE_DELAY 200000
#define BOMB_DELAY 250000

#define SLEEP_ENEMY 400000

#define ENEMY1_HEALTH 2
#define ENEMY2_HEALTH 3
#define ENEMY3_HEALTH 4

/**Comandi input**/

#define UP 65
#define DW 66
#define LT 68
#define RT 67
#define SPACEBAR ' '

/**Campo SOURCE dei vari messaggi inviati al mainLoop**/
#define ID_STARSHIP 0
// ... i numeri naturali 1:99 sono riservati ai nemici
#define ID_NEWBORN 100
#define ID_BOMB 103
#define ID_MISSILELEFT 101
#define ID_MISSILERIGHT 102
#define ID_TIMERFIRE_STARSHIP 200
#define ID_TIMERFIRE_ENEMY 201
#define ID_TIMERMOVE 203

#define CHECK_NEXT_POSITION 300

#define GO 500
#define BUMP 550
#define COLLISION 555

#define SUICIDE 666
#define QUIT 999

/**Campo LEVEL dei vari processi**/
#define STARSHIP_LEVEL 0
#define MISSILE_LEVEL 1
#define BOMB_LEVEL 1
#define ENEMY_LEVEL1 3
#define ENEMY_LEVEL2 4
#define ENEMY_LEVEL3 5
#define PUNCTIFORM_LEVEL 1

#include <stdlib.h>
#include <stdio.h>
#include <curses.h>
#include <unistd.h>
#include <time.h>

#include "space_comm.h"
#include "space_graphics.h"

/** Procedure principali dei processi **/
void mainLoop(int, int, int, int[M][2]);
void starshipLoop(int, int);
void enemyLoop(int, int, EntityParams, int);
void missileLoop(int, int, int, int);
void bombLoop(int, int, int, int, int);

/** Procedure secondarie **/
void enemyInit(int, int, EntityParams);
void missilesInit(int, int, int);
void bombInit(int, int, int, int, int);

/** Timer **/
void timerFire(int, int, int);
void timerMove(int[M][2]);

/** Altro **/
int isAlive(pid_t);
void removeEntityFromList(EntityParams[], int);
void epilogue(int, int);

/**Funzioni di controllo della collisione**/
int checkCollision(EntityParams, EntityParams[]);
int checkCoordinates(EntityParams, EntityParams);
void retrieveSizeInformation(EntityParams, int*, int*);

#endif
