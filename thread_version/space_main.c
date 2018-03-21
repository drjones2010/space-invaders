#include "space_game.h"
 
void main(){
	Coords pos; //Posizioni di partenza per le navi nemiche
	EntityParams enemyDummy;		//Informazioni iniziali per attivare l'enemyLoop
	int n;
	
	counter_mainBuffer = 0;		//Inizialmente il contatore è a zero

	pthread_mutex_init (&mutex_curses, NULL);
	pthread_mutex_init (&mutex_mainBuffer, NULL);
	pthread_mutex_init (&mutex_starshipBuffer, NULL);
	pthread_mutex_init (&mutex_entityList, NULL);
	sem_init (&semaphore_mainBuffer, 0, 0); 
	
	///Inizializza contatori, mutex e semaphore
	for (n=0; n<M; n++){
		counter_enemyBuffer[n]=0;
		counter_enemyTimerBuffer[n]=0;
		pthread_mutex_init (&mutex_enemyBuffer[n], NULL);
		pthread_mutex_init (&mutex_enemyTimerBuffer[n], NULL);
		sem_init (&semaphore_enemyBuffer[n], 0, 0); 
		sem_init (&semaphore_enemyTimerBuffer[n], 0, 0);
	}
	
	for (n=0; n<(M+1); n++)
		removeEntityFromList(n);	//Inizializzazione a -1 della entityList

	
	enemyDummy.level = ENEMY_LEVEL1;	//Livello di partenza degli enemy

		
	//Iniziamo con attivare le ncurses:
	initscr();
	noecho();
	cbreak();
	keypad(stdscr, TRUE);	//Attiva i tasti freccia sulla stdscr delle ncurses
	curs_set(0);

	 	 
	
	/** Generazione dei thread **/
	
	//Thread STARSHIP
	pthread_create (&thread_starship, NULL, &starshipLoop, NULL);
	
	//Thread ENEMY
	for (n=0; n<M; n++){
		pos = placeEnemies(n+1);
		enemyDummy.source = n+1;
		enemyDummy.x = pos.x;
		enemyDummy.y = pos.y;
		enemyDummy.parent = enemyDummy.source;
		//~ mvprintw(n, MAXX+0, "Source: %3d, X: %2d, Y: %2d", enemyDummy.source, enemyDummy.x, enemyDummy.y); refresh();
		pthread_create (&thread_enemy[n], NULL, &enemyInit, &enemyDummy);
		usleep(500);
	}

		
	mainLoop(ENTERPRISE);
	
	epilogue();
	
	endwin();
	return;
 }
 
 
 
