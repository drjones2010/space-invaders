#include "space_game.h"
 
void main(){
	 
	//Dichiarazioni
	pid_t pid_starship;
	pid_t pid_enemy[M];
	 
	int n; //Indice per i nemici
	
	int mainPipe[2];	  //Comunicazione ALL -> mainLoop 				(CONSUMATORE)
	int starshipPipe[2]; //Comunicazione mainLoop -> starshipLoop 		(PRODUTTORE)
	int enemyPipe[M][2]; //Comunicazione mainLoop -> enemy(s) 			(PRODUTTORE)
	
	Coords pos; //Posizioni di partenza per le navi nemiche
	EntityParams enemy;
		
	//Iniziamo con attivare le ncurses:
	initscr();
	noecho();
	cbreak();
	curs_set(0);
	 
	/*************** Creazione delle pipe ****************/
	 
	///MAIN
	if (createPipe(mainPipe, BLOCK) != 1) {     //DEVE rimanere bloccante, altrimenti perde messaggi!
		endwin();
		printf("Errore nella creazione della mainPipe, il programma verrà terminato.\n");
		return;
	}
	///STARSHIP
	if (createPipe(starshipPipe, UNBLOCK_R) != 1) { //Non bloccante perchè è asincrono
		endwin();
		printf("Errore nella creazione della starshipPipe, il programma verrà terminato.\n");
		return;
	}
	///ENEMIES
	for (n=0; n<M; n++){
		if (createPipe(enemyPipe[n], UNBLOCK_R) != 1) { //Non bloccante perchè è asincrono
			endwin();
			printf("Errore nella creazione della enemyPipe[%d], il programma verrà terminato.\n", n);
			return;
		}

	}
	
	/***************** Generazione dei processi *********/
	 ///STARSHIP
	 switch (pid_starship = fork()){
		case -1: perror("Errore nella fork di spaceship");
			 _exit(-1);
		
		case 0: //Processo STARSHIP
			 close(mainPipe[READ]);
			 starshipLoop(mainPipe[WRITE], starshipPipe[READ]); //Due pipe per comunicazione biunivoca tra Main e Starship
			 _exit(0);
	}
	///ENEMIES
	for (n=0; n<M; n++){
		switch (pid_enemy[n] = fork()){
			case -1: perror("Errore nella fork di enemy");
				 _exit(-1);
			
			case 0: //Processo ENEMY
				 close(mainPipe[READ]);
				 close(enemyPipe[n][WRITE]);
				 pos = placeEnemies(n+1);
				 enemy.source = n+1;
				 enemy.parent = n+1;
				 enemy.level = ENEMY_LEVEL1;
				 enemy.y = pos.y;
				 enemy.x = pos.x;
			     enemy.direction = LEFT;
				 enemyInit(mainPipe[WRITE], enemyPipe[n][READ], enemy);
				 _exit(0);
		}
	}
	
	close(mainPipe[WRITE]);
	close(starshipPipe[READ]);
	for (n=0; n<M; n++)	close(enemyPipe[n][READ]);

	mainLoop(mainPipe[READ], ENTERPRISE, starshipPipe[WRITE], enemyPipe);
	
	refresh();
	endwin();
	return;
 }
 
 
 
