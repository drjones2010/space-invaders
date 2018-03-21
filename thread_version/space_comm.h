/** Space Invaders: 8 bit attack!
 *
 * File: 
 *
 * Contiene: 
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <curses.h>
#include <pthread.h>

#ifndef SPACE_THREAD_H
#define SPACE_THREAD_H

#define COUNTER 0
#define WAIT 0
#define TRYWAIT 1
#define NOMSG 12345

#include "space_game.h"

/*Funzioni*/
void insertBuffer(EntityParams[], int*, pthread_mutex_t*, sem_t*, EntityParams);
EntityParams removeBuffer(EntityParams[], int*, pthread_mutex_t*, sem_t*, int);

/*Variabili di comunicazione*/

pthread_t thread_starship;
pthread_t thread_enemy[M];

pthread_mutex_t mutex_mainBuffer;		//Mutex di ingresso nella sezione critica del buffer
pthread_mutex_t mutex_starshipBuffer;
pthread_mutex_t mutex_enemyBuffer[M];
pthread_mutex_t mutex_enemyTimerBuffer[M];

pthread_mutex_t mutex_entityList;		//Mutex per leggere sulla entityList
pthread_mutex_t mutex_curses;			//Mutex per scrivere sullo schermo
pthread_mutex_t mutex_fprintf;			//Mutex per l'output su file

sem_t semaphore_mainBuffer;					//Semaphore che mette il consumatore del mainBuffer in attesa di riempimento
sem_t semaphore_starshipBuffer;				//Semaphore che mette il consumatore del starshipBuffer in attesa di riempimento
sem_t semaphore_enemyBuffer[M];				//Semaphore che mette il consumatore del enemyBuffer[k] in attesa di riempimento
sem_t semaphore_enemyTimerBuffer[M];		//Semaphore che mette il consumatore del enemyTimerBuffer[k] in attesa di riempimento

int counter_mainBuffer;				//Posizioni vuote rimanenti del mainBuffer
int counter_starshipBuffer;			//Posizioni vuote rimanenti dello starshipBuffer
int counter_enemyBuffer[M];			//Posizioni vuote rimanenti del enemyBuffer[k]
int counter_enemyTimerBuffer[M];	//Posizioni vuote rimanenti del enemyTimerBuffer[k]

EntityParams mainBuffer[DIM_BUFFER];			//Buffer del mainLoop, alla posizione zero viene salvato il contatore
EntityParams starshipBuffer[DIM_BUFFER];		//Buffer dell'astronave, alla posizione zero viene salvato il contatore
EntityParams enemyBuffer[M][DIM_BUFFER];		//Buffer dei nemici, la prima dimensione identifica il numero di nemico e alla posizione zero della seconda dimensione viene salvato il contatore
EntityParams enemyTimerBuffer[M][DIM_BUFFER];	//Buffer dei timer dei nemici, la prima dimensione identifica il numero di nemico e alla posizione zero della seconda dimensione viene salvato il contatore



#endif

