 #include "space_game.h"

/***********************************************************************
 * Nome: mainLoop
 * Parametri:
 * 		mainPipeRead: pipe in lettura della mainPipe
 * 		shipModel: parametro per la scelta dell'astronave giocatore
 * 		starshipPipeWrite: pipe in scrittura della starshipPipe
 * 		enemyPipe: pipe mainLoop -> enemyLoop
 * Si occupa di gestire le collisioni e la grafica delle varie entità
 * ********************************************************************/


 void mainLoop(int mainPipeRead, int shipModel, int starshipPipeWrite, int enemyPipe[][2]){
	
	pid_t pid_timerMove; 
	
	int n; //n è l'indice dei nemici
	int i, j, k;
	int entityNumber;
	int entityHit;
	int flagBump=0;					//flag per i rimbalzi degli alieni
	char c;
	int score = 0;
	int status;

	EntityParams starship;				//struttura vuota adibita al download dei dati attinenti a starship
	EntityParams missileLeft;			//struttura vuota adibita al download dei dati attinenti a missileLeft
	EntityParams missileRight;			//struttura vuota adibita al download dei dati attinenti a missileRight
	EntityParams bomb;					//Tiene traccia di tutte le bombe, come la entityList

	EntityParams entityList[M+1];		//Lista delle entità

	EntityParams newData; 				//struttura vuota adibita al download dei dati generici
	EntityParams hit;					//Struttura per l'upload delle collisioni
	
	///Inizializza i campi degli array 'entityList' e 'enemyBomb' a -1
	for (n=0; n<(M+1); n++){
		removeEntityFromList(entityList, n);
	}
	
	///Genera il timerMove (per lo spostamento verticale)
	switch(pid_timerMove=fork()){
		case -1: perror("Errore nella fork di timerMove");
			 _exit(-1);
		
		case 0: //Processo timerMove
			 timerMove(enemyPipe);
			 _exit(0);
	}
	
	///Preinizializzazioni
	entityList[ID_STARSHIP].pid=0;
	starship.x=-1;
	hit.pid = getpid();
	hit.source=-1;
	hit.level=-1;
	hit.x=-1;
	hit.y=-1;
	
	///Loop di gioco
	while ((score < M*3)&&(entityList[ID_STARSHIP].pid != -1)) {
		//Cicla fintanto che non sia stato raggiunto il punteggio massimo e il pid del giocatore sia diverso da -1
		
		///Scarica i nuovi dati dalla mainPipe
		newData = receiveMessage(mainPipeRead);
		
		///Controlla i dati
		switch(newData.source){
			
///Richiesta di terminazione da parte di una qualsiasi entità (alieni o giocatore)
			case SUICIDE:
				entityNumber = newData.parent;
				moveEntity(entityList[entityNumber].y, entityList[entityNumber].x, entityList[entityNumber].level, FALSE);	//Cancella la grafica
				removeEntityFromList(entityList, entityNumber);	//Mette a -1 tutti i campi dell'entità corrispondente
				if (entityNumber != 0) score++;	//Incrementa il punteggio (a patto che non sia il giocatore)
				break;
			
///Astronave giocatore
			case ID_STARSHIP:	//Processo spaceShip
				if ((starship.x>=0)||(starship.y>=0)){
					moveShip(shipModel, starship.y, starship.x, FALSE);
				}
				starship=newData;
				moveShip(shipModel, starship.y, starship.x, TRUE);
				entityList[ID_STARSHIP] = starship;
				entityHit = checkCollision(starship, entityList);
				//~ if (entityHit > -1){
					//~ hit.source = COLLISION;
					//~ //sendMessage(hit, enemyPipe[entityHit-1][WRITE]);
				//~ }
				break;
			
///Missile sinistro
			case ID_MISSILELEFT:	//Processo missileLeft
				missileLeft = newData;
				moveWeapon(missileLeft.source, missileLeft.y, missileLeft.x, TRUE);			//Disegna nella nuova posizione
				if(missileLeft.y != starship.y-1) //Evita di cancellare un pezzo di astronave
					moveWeapon(missileLeft.source, missileLeft.y, missileLeft.x, FALSE);	//Cancella la vecchia posizione
				
				///Controlla le collisioni
				entityHit = checkCollision(missileLeft, entityList);
				if (entityHit > -1){
					kill(missileLeft.pid, 1);						//Uccide il processo puntiforme
					mvaddch(missileLeft.y, missileLeft.x, ' ');		//Lo cancella
					refresh();
					hit.source = COLLISION;
					sendMessage(hit, enemyPipe[entityHit-1][WRITE]);
				}
				break;
				
///Missile destro
			case ID_MISSILERIGHT:	//Processo missileRight
				missileRight = newData;
				moveWeapon(missileRight.source, missileRight.y, missileRight.x, TRUE);			//Disegna nella nuova posizione
				if(missileRight.y != starship.y-1) //Evita di cancellare un pezzo di astronave
					moveWeapon(missileRight.source, missileRight.y, missileRight.x, FALSE);		//Cancella la vecchia posizione
					
				///Controlla le collisioni
				entityHit = checkCollision(missileRight, entityList);
				if (entityHit > -1){
					kill(missileRight.pid, 1);						//Uccide il processo puntiforme
					mvaddch(missileRight.y, missileRight.x, ' ');		//Lo cancella
					refresh();
					hit.source = COLLISION;
					sendMessage(hit, enemyPipe[entityHit-1][WRITE]);
				}
				break;
				
///timerFire dell'astronave
			case ID_TIMERFIRE_STARSHIP: //Processo timerFire Starship
				sendMessage(newData, starshipPipeWrite);
				break;
				
///Bomba delle astronavi nemiche
			case ID_BOMB:
				bomb = newData;
				
				if (bomb.pid != -1) //Esegue solo se ha sparato almeno una volta
					moveWeapon(bomb.source, bomb.y, bomb.x, FALSE);//Cancella la vecchia posizione
				moveWeapon(bomb.source, bomb.y, bomb.x, TRUE); //Disegna nella nuova posizione
				
				///Controlla le collisioni
				entityHit = checkCollision(bomb, entityList);
				if (entityHit == ID_STARSHIP){							//Fa il controllo solo sull'astronave del giocatore
					hit.source = COLLISION;
					sendMessage(hit, starshipPipeWrite);
					
					kill(bomb.pid, 1);									//Uccide la bomba
					mvaddch(bomb.y, bomb.x, ' ');						//La cancella
					removeEntityFromList(entityList, ID_STARSHIP);		//Pone a -1 i campi dell'astronave del giocatore
				}
				
				if (bomb.y == MAXY) mvaddch(bomb.y, bomb.x, ' ');		//Cancella la bomba nella fila più bassa una volta che raggiunge il bordo inferiore

				break;
///Astronave nemica	
             default: //processo enemy
				entityNumber = newData.source;
				
				///Cancella la vecchia posizione
                if (entityList[entityNumber].pid != -1){ //Esegue solo se l'enemy è già presente in entityList
                    moveEntity(entityList[entityNumber].y, entityList[entityNumber].x, entityList[entityNumber].level, FALSE);	//Cancella la vecchia posizione
                }
                
                ///Registra la nuova posizione
                entityList[entityNumber]=newData; //Aggiorna la lista
                
                ///Disegna la nuova posizione
                moveEntity(newData.y, newData.x, newData.level, TRUE);  //Disegna sullo schermo
                mvprintw(newData.y+1, newData.x+1, "%d", entityNumber); refresh(); //Stampa il numero di entità sopra
                
                ///Termina il programma se raggiunge il bordo inferiore
                if (entityList[entityNumber].y == MAXY-entityList[entityNumber].level) entityList[ID_STARSHIP].pid=-1;

                ///Controllo della collisione
                newData.x += newData.direction; 					//Controlla la prossima posizione
                entityHit = checkCollision(newData, entityList);	//Fa il controllo vero e proprio

                if (entityHit > 0){ 						//Fa il controllo su un'astronave alleata
                    hit.source = BUMP;
                    sendMessage(hit, enemyPipe[entityNumber-1][WRITE]);
                    sendMessage(hit, enemyPipe[entityHit-1][WRITE]);
                }
                
				if (entityHit == ID_STARSHIP){				//Fa il controllo sull'astronave del giocatore
					hit.source = COLLISION;
					sendMessage(hit, starshipPipeWrite);
					removeEntityFromList(entityList, ID_STARSHIP);
				}

				break;
			}

		usleep(500);
		refresh();
	}
	
	///Valori d'uscita
	if (entityList[ID_STARSHIP].pid == -1)
		status = FAIL;
	else status = WIN;
	
	epilogue(score, status);

}

/***********************************************************************
 * Nome: starshipLoop
 * Parametri:
 * 		mainPipeWrite: pipe in scrittura di mainPipe
 * 		starshipPipeRead: pipe in lettura di starshipPipe
 * Inizializza la struttura dell'astronave giocatore; gestisce gli input 
 * da tastiera; 
 * ********************************************************************/
 void starshipLoop(int mainPipeWrite, int starshipPipeRead){
	EntityParams starship; 		//COMM: starshipLoop -> mainLoop
	EntityParams fireOrder;		//COMM: starshipLoop <- timerFire
	EntityParams newData;		//COMM: starshipLoop <- mainLoop
	char c;

	int starshipFirePermission = ALLOWED;

	starship.pid = getpid();
	starship.source = ID_STARSHIP;
	starship.level = STARSHIP_LEVEL;
	starship.parent = ID_STARSHIP;
	starship.x = (MAXX-SSX)/2;		//Parte dal centro
	starship.y = MAXY-SSY;

	sendMessage(starship, mainPipeWrite); //Preinizializzazione delle coordinate


	while(1) {
		newData = receiveMessage(starshipPipeRead);

		///Accetta l'input da tastiera
		switch(c=fgetc(stdin)) {	//Controlli verticali disabilitati di default
			/*case UP:		// Sposta su
				if (starship.y>0) starship.y-=1;
				break;
			case DW:		// Sposta giu
				if (starship.y<MAXY-1) starship.y+=1;
				break;*/
			case LT:		//Sposta a sinistra
				if (starship.x>0) {
					starship.x-=1;
				}
				break;
			case RT:		//Sposta a destra
				if (starship.x<MAXX-SSX) {
					starship.x+=1;
				}
				break;
			case SPACEBAR:
				if (starshipFirePermission != DENIED) missilesInit(starship.y, starship.x, mainPipeWrite);
				break;
				
			case 'q':
				starship.source = SUICIDE;
				sendMessage(starship, mainPipeWrite);
				break;

		}
		
		///Riceve info sulla collisione e il permesso di aprire il fuoco
		switch(newData.source){
			case ID_TIMERFIRE_STARSHIP:
				fireOrder = newData;
				switch(fireOrder.level){
					case ALLOWED: starshipFirePermission = ALLOWED; break;
					case DENIED:  starshipFirePermission = DENIED; break;
				}
				break;
			case COLLISION:
				starship.source = SUICIDE;
				sendMessage(starship, mainPipeWrite);
				break;
		}

		sendMessage(starship, mainPipeWrite); //Invia le nuove coordinate alla pipe
	}
}

/***********************************************************************
 * Nome: missilesInit
 * Parametri:
 * 		y,x: coordinate della navicella che ha generato i missili
 * 		mainPipeWrite: pipe in scrittura di mainPipe
 * Crea, tramite fork(), i due missili e un timerFire, che serve a 
 * impedire la generazione di altri missili fino alla propria morte.
 * ********************************************************************/
void missilesInit(int y, int x, int mainPipeWrite){
	pid_t pid_missileLeft;
	pid_t pid_missileRight;
	pid_t pid_timerFire;

	///Genera un nuovo timerFire ogni volta che viene aperto il fuoco
	switch (pid_timerFire = fork()){
		case -1: //Errore
			perror("Errore nella fork di timerFire");
			_exit(-1);
		case 0: //PROCESSO TIMERFIRE
				timerFire(mainPipeWrite, ID_TIMERFIRE_STARSHIP, FIRE_DELAY_STARSHIP);
				exit(0);
				break;
	}
	
	///Genera il missileLeft
	switch (pid_missileLeft = fork()){
		case -1: perror("Errore nella fork di missileLeft");
		_exit(-1);

		case 0: //Processo MISSILELEFT
			missileLoop(ID_MISSILELEFT, y, x, mainPipeWrite);
			_exit(0);
			break;
	}
	
	///Genera il missileRight
	switch (pid_missileRight = fork()){
		case -1: perror("Errore nella fork di missileRight");
		_exit(-1);

		case 0: //Processo MISSILERIGHT
			missileLoop(ID_MISSILERIGHT, y, x, mainPipeWrite);
			_exit(0);
	}

	return;
}
/**********************************************************************
 * Nome: bombInit
 * Parametri:
 * 		parentNumber: il numero dell'enemy che ha creato la bomba
 * 		parentLevel: il livello dell'enemy che ha creato la bomba
 * 		y, x: coordinate dell'enemy che ha creato la bomba
 * 		mainPipeWrite: pipe su cui scrivere
 * La funzione crea una bomba, tramite fork()
 * *******************************************************************/
void bombInit(int parentNumber, int parentLevel, int y, int x, int mainPipeWrite){
	pid_t pid_bomb;
	
	///Genera la bomba
	switch (pid_bomb = fork()){
		case -1: perror("Errore nella fork di bomb");
		_exit(-1);

		case 0: //Processo BOMB
			bombLoop(parentNumber, parentLevel, y, x, mainPipeWrite);
			_exit(0);
	}

	return;
}

/***********************************************************************
 * Nome: missileLoop
 * Parametri:
 * 		id: l'id del missile, può assumere due valori, a seconda che si 
 * 			tratti del missile destro o del missile sinistro
 * 		y, x: coordinate della navicella che ha generato i missili
 * 		mainPipeWrite: pipe in scrittura di mainPipe
 * Si occupa di determinare le coordiante iniziali e i successivi 
 * spostamenti dei missili.
 * ********************************************************************/
void missileLoop(int id, int y, int x, int mainPipeWrite){
	EntityParams missile;

	missile.source = id;
	missile.y = y-1;
	missile.pid = getpid();
	missile.level = MISSILE_LEVEL;

	if (id == ID_MISSILELEFT){
		missile.x = x+2;	//Centra la posizione di partenza del missile rispetto all'astronave
		sendMessage(missile, mainPipeWrite);
		do{ ///Loop vitale
			missile.x--;
			missile.y--;
			sendMessage(missile, mainPipeWrite);
			usleep(MISSILE_DELAY);
		  //Cicla fintanto che il missile si trova all'interno dell'area di MAXX*MAXY
		} while(((missile.x >= 0) && (missile.x < MAXX)) && ((missile.y >= 0) && (missile.y < MAXY)));
	}

	if (id == ID_MISSILERIGHT){
		missile.x = x+3;	//Centra la posizione di partenza del missile rispetto all'astronave
		sendMessage(missile, mainPipeWrite);
		do{ ///Loop vitale
			missile.x++;
			missile.y--;
			sendMessage(missile, mainPipeWrite);
			usleep(MISSILE_DELAY);
		  //Cicla fintanto che il missile si trova all'interno dell'area di MAXX*MAXY
		} while(((missile.x >= 0) && (missile.x < MAXX)) && ((missile.y >= 0) && (missile.y < MAXY)));
	}

	_exit(0);
}
/***********************************************************************
 * Nome: bombLoop
 * Parametri:
 * 		parentNumber: il numero dell'enemy che ha creato la bomba
 * 		parentLevel: il livello dell'enemy che ha creato la bomba
 * 		y, x: coordinate dell'enemy che ha creato la bomba
 * 		mainPipeWrite: pipe su cui scrivere
 * La funzione bombLoop, si occupa di posizionare la bomba a seconda del 
 * livello dell'enemy, nonchè dello spostamento e della terminazione della 
 * bomba stessa
 * ********************************************************************/
void bombLoop(int parentNumber, int parentLevel, int y, int x, int mainPipeWrite){
	EntityParams bomb;		//COMM: bombLoop -> mainLoop

	///Inizializzazione preliminare
	bomb.source = ID_BOMB;
	bomb.pid = getpid();
	bomb.level = BOMB_LEVEL;
	bomb.parent = parentNumber;
	bomb.x = x;
	
	///Posizione di partenza della bomba in relazione al livello del nemico
	switch (parentLevel){
		case ENEMY_LEVEL1:
			bomb.y = y+3;
			bomb.x+=1;
			break;
		case ENEMY_LEVEL2:
			bomb.y = y+4;
			bomb.x+=1;
			break;
		case ENEMY_LEVEL3:
			bomb.y = y+5;
			bomb.x+=2;
			break;
	}
	
	sendMessage(bomb, mainPipeWrite); //Preinizializzazione delle coordinate
	
	
	///Loop vitale
	do{ 
		usleep(BOMB_DELAY);
		bomb.y++;
		sendMessage(bomb, mainPipeWrite);
	} while(bomb.y < MAXY);

	_exit(0);
}

/***********************************************************************
 * Nome: timerFire
 * Parametri: 
 * 		pipeWrite: pipe in scrittura
 * 		id: id di chi ha invocato timerFire
 * 		delay: ritardo richiesto dall'entità che ha generato timerFire
 * Gestisce gli intervalli tra gli spari
 * ********************************************************************/
void timerFire(int pipeWrite, int id, int delay){
	EntityParams order;	//COMM: timerFire -> mainLoop/enemyLoop


	
	///Inizializzazione preliminare
	order.pid=getpid();
	order.source = id;
	order.level = DENIED;
	order.x = -1;
	order.y = -1;
	
	switch (id){
		
		///timerFire dell'astronave, regola la pressione della barra spaziatrice
		case ID_TIMERFIRE_STARSHIP:
			sendMessage(order, pipeWrite);
			usleep(delay);
			order.level = ALLOWED;
			sendMessage(order, pipeWrite);
			_exit(0);
			break;
		
		///timerFire dell'astronave nemica
		default:
			while(isAlive(getppid())){
				order.level = ALLOWED;
				sendMessage(order, pipeWrite);
				order.level = DENIED;
				sendMessage(order, pipeWrite);
				sleep(delay);
			};
			_exit(0);
			break;
	};

	_exit(0);
}

/***********************************************************************
 * Nome: timerMove
 * Parametri:
 * 		enemyPipe: array di pipe
 * Crea un timer per il movimento verticale delle astronavi nemiche 
 * ********************************************************************/
void timerMove(int enemyPipe[M][2]){
	EntityParams timer;		//COMM: timerMove -> enemyLoop
	int i;
	timer.source = ID_TIMERMOVE;
	timer.pid=getpid();
	timer.x=-1;
	timer.y=-1;
	timer.level = -1;

	while (getppid()){
		sleep(MOVE_Y_DELAY);
		for(i=0; i<M; i++) sendMessage(timer, enemyPipe[i][WRITE]);
	}
	_exit(0);

}

int isAlive(pid_t pid){
	if ((int)pid > 1) return 1;
	else return 0;
}

/***********************************************************************
 * Nome: enemyInit
 * Parametri:
 * 		thisEntity: struttura dell'entità considerata
 * La funzione esamina il livello dell'entità presa in considerazione e 
 * invoca enemyLoop con i parametri adeguati alla nave dello specifico 
 * livello
 * ********************************************************************/

void enemyInit(int mainPipeWrite, int enemyPipeRead, EntityParams thisEntity){
	
	switch(thisEntity.level){
		case ENEMY_LEVEL1:
			enemyLoop(mainPipeWrite, enemyPipeRead, thisEntity, ENEMY1_HEALTH);
			break;
		case ENEMY_LEVEL2:
			enemyLoop(mainPipeWrite, enemyPipeRead, thisEntity, ENEMY2_HEALTH);
			break;
		case ENEMY_LEVEL3:
			enemyLoop(mainPipeWrite, enemyPipeRead, thisEntity, ENEMY3_HEALTH);
			break;
	}

}

/***********************************************************************
 * Nome: enemyLoop
 * Parametri:
 * 		thisEntity: la struttura dell'entità
 * 		health: numero di vite di thisEntity
 * La funzione si occupa di quel che riguarda il ciclo di vita delle 
 * astronavi nemiche. 
 * ********************************************************************/

void enemyLoop(int mainPipeWrite, int enemyPipeRead, EntityParams thisEntity, int health) {
	pid_t pid_nextLevelEnemy;
	pid_t pid_timerFireEnemy;

	int timerFireEnemyPipe[2];		//Pipe fuoco

	//Messaggi in ingresso
	EntityParams mainData;			//COMM: enemyLoop <- mainLoop
	EntityParams timerFireData;		//COMM: enemyLoop <- timerFire
	EntityParams timerMoveData;		//COMM: enemyLoop <- timerMoveData
	
	int firePermission = DENIED;
	int movePermission = DENIED;
	
	srand(time(NULL));
	
	///Creazione della pipe per comunicare con il timer delle bombe
	if (createPipe(timerFireEnemyPipe, UNBLOCK_R) != 1) {
		endwin();
		printf("Errore nella creazione della timerFireEnemyPipe, il programma verrà terminato.\n");
		return;
	};

	///Generazione del timerFire
	switch (pid_timerFireEnemy = fork()){
			case -1: perror("Errore nella fork di timerFireEnemy");
				 _exit(-1);

			case 0: //Processo TIMERFIREENEMY
				 sleep(thisEntity.source);
				 close(timerFireEnemyPipe[READ]);
				 timerFire(timerFireEnemyPipe[WRITE], thisEntity.source, FIRE_DELAY_ENEMY1);
				 _exit(0);
			
			default:
				close(timerFireEnemyPipe[WRITE]);
	}
	
	thisEntity.pid = getpid();
	
	/********** Ciclo principale ************/
	while(health>0) {
		
		///Scarica i dati
		timerFireData = receiveMessage(timerFireEnemyPipe[READ]);
		firePermission = timerFireData.level;

		///Controlla i muri laterali
		if ((thisEntity.x == 0) || (thisEntity.x==MAXX-thisEntity.level)) //Controllo dei muri laterali
			thisEntity.direction *= -1;
		
		///Scarica i dati
		mainData = receiveMessage(enemyPipeRead);

		switch (mainData.source){
			case ID_TIMERMOVE:
				thisEntity.y++;	//Se ha ricevuto il messaggio dal timer incrementa la coordinata y
				break;
				
			case BUMP:
				if (!thisEntity.bouncing){
					thisEntity.direction *= -1;
					thisEntity.bouncing = TRUE;
				}
				break;
				
			case COLLISION:
				health--;
				break;
				
			default:
				thisEntity.bouncing = FALSE;
				break;

		}
		
		///Aggiorna la posizione
		thisEntity.x += thisEntity.direction;	//Incrementa/decrementa la posizione in x

		
		///Apre il fuoco
		if (firePermission == ALLOWED){
			bombInit(thisEntity.source, thisEntity.level, thisEntity.y, thisEntity.x, mainPipeWrite);
			firePermission = DENIED;
		}
		
		///Invia le coordinate al mainLoop
		sendMessage(thisEntity, mainPipeWrite);	//Invia le coordinate al mainLoop

		usleep(SLEEP_ENEMY);
		
	}
	/**********************/
	
	thisEntity.source = SUICIDE;
	sendMessage(thisEntity, mainPipeWrite);
	
	///Crea una nave di livello superiore
	if (thisEntity.level<ENEMY_LEVEL3){
		switch (pid_nextLevelEnemy = fork()){
			case -1: perror("Errore nella fork di spaceship");
				 _exit(-1);

			case 0: //Processo enemyLevel2
				thisEntity.source = thisEntity.parent; //Ripristina il corretto numero di alieno
				thisEntity.level++;
				enemyInit(mainPipeWrite, enemyPipeRead, thisEntity);
				break;
		}
	}

	_exit(0);

}
/***********************************************************************
 * Nome: checkCollision
 * Parametri in ingresso:
 * 		thisEntity: struttura dell'entità considerata
 * 		entityList[]: la lista delle entità contro cui thisEntity potrebbe
 * 					collidere, esclusi missili e bombe
 * La funzione scorre tutta la entityList e passa le strutture thisEntity
 * e entityList[i] alla funzione checkCoordinates, che si occuperà del 
 * controllo vero e proprio. La funzione restituisce il valore dell'entità 
 * su cui thisEntity ha colliso, altrimenti restituisce -1
 * ********************************************************************/

int checkCollision(EntityParams thisEntity, EntityParams entityList[]){
	int i, j, k;
	int value=-1;

	for (i=0; i<(M+1); i++){
		if (i != thisEntity.source)
			value = checkCoordinates(thisEntity, entityList[i]);
		if (value != -1) return value;
	}
	//~ if (value!=-1) mvprintw(MAXY/2, MAXX+0, "checkCollision: value = %2d", value); refresh(); //Debugging
	
	return value; //Non ha trovato corrispondenze

}
/***********************************************************************
 * Nome: checkCoordinates
 * Parametri in ingresso:
 * 		thisEntity: struttura dell'entità considerata
 * 		otherEntity: struttura della seconda entità ccon cui fare il confronto
 * Parametri in uscita:
 * 		int
 * La funzione prende in ingresso due entità, controlla se le due entità
 * si sovrappongono e, in caso di collisione, restituisce il source dell'entità 
 * su cui thisEntity ha colliso, altrimenti restituisce -1
 * ********************************************************************/
int checkCoordinates(EntityParams thisEntity, EntityParams otherEntity){

	int j, k, n, m;
	int thisEntitySizeX;
	int thisEntitySizeY;
	retrieveSizeInformation(thisEntity, &thisEntitySizeX, &thisEntitySizeY); //Ha effetto sulle variabili qua sopra
	int otherEntitySizeX;
	int otherEntitySizeY;
	retrieveSizeInformation(otherEntity, &otherEntitySizeX, &otherEntitySizeY); //Ha effetto sulle variabili qua sopra


		for (j=0; j<thisEntitySizeY; j++){	//Cicla le righe di thisEntity
			for (k=0; k<thisEntitySizeX; k++){	//Cicla le colonne di thisEntity
				for (n=0; n<otherEntitySizeY; n++){		//Cicla le righe di otherEntity
					for (m=0; m<otherEntitySizeX; m++){		//Cicla le colonne di otherEntity
						if ((thisEntity.x+k == otherEntity.x+m) && (thisEntity.y+j == otherEntity.y+n)){
							//~ mvprintw(MAXY+1, 0, "checkCoordinates: Entity(%3d) ha colliso su Entity(%3d)", thisEntity.source, otherEntity.source);  refresh(); //Debugging
							
							return otherEntity.source;
						}

					}
				}
			}
		}

	return -1;
	
}

/***********************************************************************
 * Nome: retrieveSizeInformation
 * Parametri:
 * 		thisEntity: la struttura dell'entità presa in esame
 * 		*thisEntitySizeX: puntatore alla cella di memoria che contiene 
 * 						la larghezza dell'entità
 * 		*thisEntitySizeY: puntatore alla cella di memoria che contiene 
 * 						l'altezza dell'entità
 * La funzione prende in ingresso l'entità e, a seconda del livello, 
 * modifica i valori contenuti in *thisEntitySizeX e thisEntitySizeY
 * ********************************************************************/

void retrieveSizeInformation(EntityParams thisEntity, int* thisEntitySizeX, int* thisEntitySizeY){
	 switch (thisEntity.level){
		 case STARSHIP_LEVEL:
			*thisEntitySizeX = SSX;
			*thisEntitySizeY = SSY;
			break;
		 case ENEMY_LEVEL1:
			*thisEntitySizeX = E1X;
			*thisEntitySizeY = E1Y;
			break;
		 case ENEMY_LEVEL2:
			*thisEntitySizeX = E2X;
			*thisEntitySizeY = E2Y;
			break;
		 case ENEMY_LEVEL3:
			*thisEntitySizeX = E3X;
			*thisEntitySizeY = E3Y;
			break;
		 case PUNCTIFORM_LEVEL:
			*thisEntitySizeX = 1;
			*thisEntitySizeY = 1;
			break;
	 }

}

/***********************************************************************
 * Nome: removeEntityFromList
 * Parametri: 
 * 		array[]: array di entità
 * 		n: numero dell'entità considerata
 * Porta tutti i valori dei campi di entityparams a -1
 * ********************************************************************/
void removeEntityFromList(EntityParams array[], int n){
	array[n].source = -1;
	array[n].pid = -1;
	array[n].level = -1;
	array[n].x = -1;
	array[n].y = -1;
	array[n].parent = -1;
	array[n].direction = -1;
}

/***********************************************************************
 * Nome: epilogue
 * Parametri:
 * 		score: numero di entità enemy uccise nel gioco
 * 		status: è uguale a FAIL se la navicella del giocatore è stata uccisa,
 * 				WIN altrimenti
 * ********************************************************************/

void epilogue(int score, int status){
	double punteggio = (score*3*1000)/5;
	
	erase();
	
	if (status == WIN)
		mvprintw(MAXY/2, MAXX/2, "Hai vinto! Il tuo punteggio è %6.2f", punteggio); 
		
	else mvprintw(MAXY/2, MAXX/2, "Hai perso! Il tuo punteggio è %6.2f", punteggio);
	
	refresh();
	
	sleep(8);
}
