#ifndef SPACE_MAIN_H
#define SPACE_MAIN_H

#include <stdlib.h>
#include <stdio.h>
#include <curses.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>

/**Varie ed eventuali**/

#define M 4	//Numero dei nemici
#define DIM_BUFFER 10000

#define TRUE 1
#define FALSE 0

#define FAIL 0
#define WIN 1

#define STARSHIP 1
#define ALL 0

#define LEFT -1
#define RIGHT 1
#define DOWN 0

#define MAXX 80
#define MAXY 24
#define READ 0
#define WRITE 1

#define ALLOWED 1
#define DENIED 0

#define VERTICAL_DISTANCE 7

#define FIRE_DELAY_STARSHIP 500000
#define FIRE_DELAY_ENEMY 6

#define MOVE_Y_DELAY 8 //in secondi

#define MISSILE_DELAY 200000
#define BOMB_DELAY 200000

#define SLEEP_ENEMY1 500000
#define SLEEP_ENEMY2 500000
#define SLEEP_ENEMY3 500000

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
#define ID_MISSILELEFT 101   
#define ID_MISSILERIGHT 102
#define ID_BOMB 103

#define GO 500
#define BUMP 550
#define COLLISION 555
#define FIRE 777
#define MOVE 778

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

typedef struct{
	pthread_t tr;		//Thread id del mittente
	int source;			//Identità del mittente (MACRO)
	int level;			//Livello di difficoltà (0 per l'astronave, 1:3 per i nemici)
	int x;				//Coordinate del mittente
	int y;
	int direction;		//LEFT, RIGHT, DOWN
	int parent;			//Usato dai messaggi in cui source è usato per un altro tipo di ID
	int bouncing;		//Variabile per il rimbalzo
} EntityParams;


#include "space_comm.h"
#include "space_graphics.h"


//Variabili globali

FILE *stream;
int score;
int status;
EntityParams entityList[M+1];		//Lista delle entità
EntityParams starship_global;
int starshipFirePermission;
int global_y_position;

///////////////////

/** Procedure loop dei task **/
void mainLoop(int);
void* starshipLoop();
void enemyLoop(EntityParams, int, int);
void* missileLoop(void*);
void* bombLoop(void*);

/** Procedure d'inizializzazione **/
void bombInit(int);
void* enemyInit(void*);
void missilesInit();

/** Timer **/
void* timerFire(void*);
void* timerMove(void*);

/**Funzioni secondarie**/
int checkCollision(EntityParams, EntityParams[]);
int checkCoordinates(EntityParams, EntityParams);
void retrieveSizeInformation(EntityParams, int*, int*);
void removeEntityFromList(int);
void epilogue();

#endif
