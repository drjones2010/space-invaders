 #include "space_game.h"

/***********************************************************************
 * Nome: mainLoop
 * Parametri:
 * 		shipModel: parametro per la scelta dell'astronave giocatore
 * Si occupa di gestire le collisioni e la grafica delle varie entità
 * ********************************************************************/
 
void mainLoop(int shipModel){

	pthread_t thread_timerMove; 
	pthread_t thread_timerFire; 
	
	int n; //n è l'indice dei nemici
	int i, j, k;
	int entityNumber;
	int entityHit;

	char c;
	int exit = 0;
	
	EntityParams newData;
	EntityParams starship;
	EntityParams missileLeft;
	EntityParams missileRight;
	EntityParams enemy;
	EntityParams bomb;

	starship.x=-1;
	starship.y=-1;
	
	entityList[ID_STARSHIP].tr = 0;	//Evita che il gioco termini perchè non ha ricevuto ancora le coordinate dell'astronave
	
	///Creazione del thread del movimento verticale
	pthread_create(&thread_timerMove, NULL, &timerMove, NULL);
	
	while ((score < M) && (entityList[ID_STARSHIP].tr != -1)){
		newData = removeBuffer(mainBuffer, &counter_mainBuffer, &mutex_mainBuffer, &semaphore_mainBuffer, WAIT);	//Scarica nuovi messaggi dal buffer

		switch(newData.source){
///Richiesta di terminazione
			case SUICIDE:
				entityNumber = newData.parent;
				moveEntity(entityList[entityNumber].y, entityList[entityNumber].x, entityList[entityNumber].level, FALSE);	//Cancella la grafica
				removeEntityFromList(entityNumber);	//Mette a -1 tutti i valori
				if (newData.level == ENEMY_LEVEL3) score++;	//Incrementa deathCounter
				
				break;
///Astronave del giocatore
			case ID_STARSHIP:
			
				if ((starship.x>=0)||(starship.y>=0)){
					moveShip(shipModel, starship.y, starship.x, FALSE);
				}
				starship=newData;

				moveShip(shipModel, starship.y, starship.x, TRUE);
				entityList[ID_STARSHIP] = starship;

				break;
///Missile di sinistra
			case ID_MISSILELEFT:	//Processo missileLeft
				missileLeft = newData;
				if(missileLeft.y != starship.y-1) //WORKAROUND che cancella il missile che rimane attaccato allo scafo
					moveWeapon(missileLeft.source, missileLeft.y, missileLeft.x, TRUE);			//Disegna nella nuova posizione
				if(missileLeft.y != starship.y-1) //Evita di cancellare un pezzo di astronave
					moveWeapon(missileLeft.source, missileLeft.y, missileLeft.x, FALSE);	//Cancella la vecchia posizione
				
				entityHit = checkCollision(missileLeft, entityList);
				if (entityHit > -1){
					pthread_cancel(missileLeft.tr);					//Uccide il thread puntiforme
					pthread_mutex_lock(&mutex_curses);
						mvaddch(missileLeft.y, missileLeft.x, ' ');		//Lo cancella
						refresh();
					pthread_mutex_unlock(&mutex_curses);
					///Invia messaggio a all'entità colpita
					newData.source = COLLISION;
					insertBuffer(enemyBuffer[entityHit-1], &counter_enemyBuffer[entityHit-1], &mutex_enemyBuffer[entityHit-1], &semaphore_enemyBuffer[entityHit-1], newData);
				}
				break;
				
///Missile di destra
			case ID_MISSILERIGHT:	//Processo missileRight
				missileRight = newData;
				if(missileRight.y != starship.y-1) //WORKAROUND che cancella il missile che rimane attaccato allo scafo
					moveWeapon(missileRight.source, missileRight.y, missileRight.x, TRUE);
				if(missileRight.y != starship.y-1) //Evita di cancellare un pezzo di astronave
					moveWeapon(missileRight.source, missileRight.y, missileRight.x, FALSE);
				
				entityHit = checkCollision(missileRight, entityList);
				if (entityHit > -1){
					pthread_cancel(missileRight.tr);					//Uccide il thread puntiforme
					pthread_mutex_lock(&mutex_curses);
						mvaddch(missileRight.y, missileRight.x, ' ');		//Lo cancella
						refresh();
					pthread_mutex_unlock(&mutex_curses);
					
					///Invia messaggio a all'entità colpita
					newData.source = COLLISION;
					insertBuffer(enemyBuffer[entityHit-1], &counter_enemyBuffer[entityHit-1], &mutex_enemyBuffer[entityHit-1], &semaphore_enemyBuffer[entityHit-1], newData);
				}
				break;
///Bomba
			case ID_BOMB:	//Processo missileRight
				bomb = newData;
				bomb.level = BOMB_LEVEL;
				moveWeapon(bomb.source, bomb.y, bomb.x, TRUE);
				moveWeapon(bomb.source, bomb.y, bomb.x, FALSE);
				
				///Controlla le collisioni
				entityHit = checkCollision(bomb, entityList);
				
				if (entityHit == ID_STARSHIP){ //Controlla se ha colpito l'astronave del giocatore
					bomb.source = COLLISION;
					insertBuffer(starshipBuffer, &counter_starshipBuffer, &mutex_starshipBuffer, &semaphore_starshipBuffer, bomb);
					entityList[ID_STARSHIP].tr=-1;	//Non aspetta che lo starshipLoop termini.
					
					//Cancella la bomba
					pthread_cancel(bomb.tr);				//Uccide il thread puntiforme
					pthread_mutex_lock(&mutex_curses);
						mvaddch(bomb.y, bomb.x, ' ');		//Lo cancella
						refresh();
					pthread_mutex_unlock(&mutex_curses);
				}
				if (bomb.y == MAXY-1){
					pthread_mutex_lock(&mutex_curses);
					mvaddch(bomb.y, bomb.x, ' '); refresh();		//Cancella la bomba nella fila più bassa una volta che raggiunge il bordo inferiore
					pthread_mutex_unlock(&mutex_curses);
				}
				break;
			
///Astronave nemica
			default:
				enemy = newData;
				entityNumber = newData.parent;

				///Cancella vecchia posizione
                if (entityList[entityNumber].tr != -1){ //Esegue la cancellazione alla posizione precedente solo se l'enemy è già presente in entityList
                    moveEntity(entityList[entityNumber].y, entityList[entityNumber].x, entityList[entityNumber].level, FALSE);
                }
                
                ///Registra nuova posizione
                entityList[entityNumber]=newData; //Aggiorna la entityList
                
                ///Scrivi nuova posizione
                moveEntity(enemy.y, enemy.x, enemy.level, TRUE);  //Disegna sullo schermo
                pthread_mutex_lock(&mutex_curses);
                mvprintw(enemy.y+1, enemy.x+1, "%d", enemy.source); //Stampa il numero di processo sopra l'astronave
                pthread_mutex_unlock(&mutex_curses);
                
                ///Termina il programma se raggiunge il bordo inferiore
                if (enemy.y == MAXY-enemy.level) entityList[ID_STARSHIP].tr=-1;
                
                ///Controllo della collisione
                if (((newData.x + newData.direction*1) >= 0) && ((newData.x + newData.direction*4) <= MAXX)) newData.x += newData.direction*1; 	//Lascia uno spazio bianco prima della collisione
                entityHit = checkCollision(newData, entityList);	//Fa il controllo vero e proprio (tramite entityList)
                
                if (entityHit > 0){ //Controlla la collisione sulle navi alleate
                    newData.source = BUMP;
                    insertBuffer(enemyBuffer[entityHit-1], &counter_enemyBuffer[entityHit-1], &mutex_enemyBuffer[entityHit-1], &semaphore_enemyBuffer[entityHit-1], newData);
                    insertBuffer(enemyBuffer[entityNumber-1], &counter_enemyBuffer[entityNumber-1], &mutex_enemyBuffer[entityNumber-1], &semaphore_enemyBuffer[entityNumber-1], newData);
                }
                
                if (entityHit == ID_STARSHIP){ //Controlla la collisione sul giocatore
                    newData.source = COLLISION;
                    insertBuffer(starshipBuffer, &counter_starshipBuffer, &mutex_starshipBuffer, &semaphore_starshipBuffer, newData);
                    entityList[ID_STARSHIP].tr=-1;	//Non aspetta che lo starshipLoop termini.
                    removeEntityFromList(ID_STARSHIP);
                }

				break;

		}		
		
		
		//~ pthread_mutex_lock(&mutex_curses);
		usleep(500);
		//~ for (i=0; i<M+1; i++){
			//~ //Stampa la lista delle entità
			//~ mvprintw(i, MAXX, "EntityList: %3d | %3d | %3d | %lu", entityList[i].source, entityList[i].x, entityList[i].y, (int)entityList[i].tr);
			//~ 
		//~ }
		
		//~ refresh();	//Minimizza i bug grafici
		//~ pthread_mutex_unlock(&mutex_curses);
		
		
	} 
	
	if (entityList[ID_STARSHIP].tr == -1) status=FAIL;
		else status=WIN;
	

}

/***********************************************************************
 * Nome: starshipLoop
 * Parametri:
 * 		Nessuno
 * Inizializza la struttura dell'astronave giocatore; gestisce gli input 
 * da tastiera; 
 * ********************************************************************/
void* starshipLoop(void* arg){
	EntityParams starship; //Struttura vuota in cui scaricare i dati
	EntityParams mainData;

	pthread_t thread_timerFire;
	
	int c;
	int health = 1;
	
	starshipFirePermission = ALLOWED;
	 
	starship.tr = pthread_self();
	starship.source = ID_STARSHIP;
	starship.level = STARSHIP_LEVEL;
	starship.x = (MAXX-SSX)/2;		//Parte dal centro
	starship.y = MAXY-SSY;
	
	
	while(health > 0) {
		starship_global=starship;
		insertBuffer(mainBuffer, &counter_mainBuffer, &mutex_mainBuffer, &semaphore_mainBuffer, starship);
		
		switch(c=getch()) {	//Controlli verticali disabilitati di default
/*			case KEY_UP:		// Sposta su
				if (starship.y>0) starship.y-=1;
				break;
			case KEY_DOWN:		// Sposta giu
				if (starship.y<MAXY-1) starship.y+=1;
				break;
*/			case KEY_LEFT:		//Sposta a sinistra
				if (starship.x>0) {
					starship.x-=1;
				}				
				break;
			case KEY_RIGHT:		//Sposta a destra
				if (starship.x<MAXX-SSX) {
					starship.x+=1;
				}
				break;
			case SPACEBAR:
				if (starshipFirePermission != DENIED){
					pthread_create (&thread_timerFire, NULL, &timerFire, (void*)ID_STARSHIP); //Serve a rallentare il rateo di fuoco
					missilesInit();
				}
		}
		
		mainData = removeBuffer(starshipBuffer, &counter_starshipBuffer, &mutex_starshipBuffer, &semaphore_starshipBuffer, TRYWAIT); //La trywait() consente un'attesa non bloccante

		switch (mainData.source){
			case COLLISION:
				health--;
				//~ mvprintw(10, MAXX+0, "colpito! HP:%d.", health); refresh(); //DEBUG
				/// Richiesta di eliminazione e cancellazione
				starship.source = SUICIDE;	//Informa il mainLoop che si sta per terminare, in modo tale da cancellare la propria registrazione nella entityList
				insertBuffer(mainBuffer, &counter_mainBuffer, &mutex_mainBuffer, &semaphore_mainBuffer, starship);
				break;
		}
		
	} 
}
/***********************************************************************
 * Nome: enemyInit
 * Parametri:
 * 		arg: struttura dell'entità considerata (necessita casting)
 * La funzione esamina il livello dell'entità presa in considerazione e 
 * invoca enemyLoop con i parametri adeguati alla nave dello specifico 
 * livello
 * ********************************************************************/
void* enemyInit(void* arg) {
	
	EntityParams thisEntity = *((EntityParams*) arg);	//Casting del parametro

	switch(thisEntity.level){
		case ENEMY_LEVEL1:
			enemyLoop(thisEntity, ENEMY1_HEALTH, SLEEP_ENEMY1);
			break;
		case ENEMY_LEVEL2:
			enemyLoop(thisEntity, ENEMY2_HEALTH, SLEEP_ENEMY1);
			break;
		case ENEMY_LEVEL3:
			enemyLoop(thisEntity, ENEMY3_HEALTH, SLEEP_ENEMY1);
			break;
		}
}
/***********************************************************************
 * Nome: enemyLoop
 * Parametri:
 * 		thisEntity: la struttura dell'entità
 * 		health: numero di vite di thisEntity
 * 		sleep: il ritardo associato
 * La funzione si occupa di quel che riguarda il ciclo di vita delle 
 * astronavi nemiche. 
 * ********************************************************************/
void enemyLoop(EntityParams thisEntity, int health, int sleep){

	
	pthread_t thread_enemyNext;
	pthread_t thread_timerFire;
	EntityParams mainData;			//Scarica i dati ricevuti dal buffer personale con il mainLoop
	EntityParams timerData;		//Scarica i dati ricevuti dal buffer personale con i timer

	int startY=(thisEntity.y/VERTICAL_DISTANCE)*VERTICAL_DISTANCE;

	int firePermission = ALLOWED;
	int movePermission = DENIED;

	thisEntity.tr = pthread_self();		//Prende l'id del thread corrent

	thisEntity.direction = LEFT; //Inizializzazione in caso il random non funzioni

	
	///Creazione del thread del fuoco
	pthread_create(&thread_timerFire, NULL, &timerFire, (void*)thisEntity.source);

	/********** Ciclo Vitale***********/
	while(health>0) {
				
		///CHECK: Muri laterali
		if ((thisEntity.x == 0) || (thisEntity.x==MAXX-thisEntity.level)) //Controllo dei muri laterali
			thisEntity.direction *= -1;
			
		///Controlla i messaggi dal mainLoop
		mainData = removeBuffer(enemyBuffer[thisEntity.source-1], &counter_enemyBuffer[thisEntity.source-1], &mutex_enemyBuffer[thisEntity.source-1], &semaphore_enemyBuffer[thisEntity.source-1], TRYWAIT); //La trywait() consente un'attesa non bloccante
		timerData = removeBuffer(enemyTimerBuffer[thisEntity.source-1], &counter_enemyTimerBuffer[thisEntity.source-1], &mutex_enemyTimerBuffer[thisEntity.source-1], &semaphore_enemyTimerBuffer[thisEntity.source-1], TRYWAIT); //La trywait() consente un'attesa non bloccante	

		///CHECK: Lettura messaggio
		switch (mainData.source){				
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
		
		///CHECK: fuoco
		if (timerData.source == FIRE)
			bombInit(thisEntity.source);

		///Aggiorna le coordinate X,Y secondo la sua direzione
		thisEntity.x += thisEntity.direction;
		thisEntity.y = startY + global_y_position;
		
		///Scrive nel mainBuffer le nuove coordinate
		insertBuffer(mainBuffer, &counter_mainBuffer, &mutex_mainBuffer, &semaphore_mainBuffer, thisEntity);
		
		///Sleep
		usleep(sleep);
		

	}
	/******FINE DEL CICLO**************/
	
	/// Richiesta di eliminazione e cancellazione dalla entityList per fare spazio a una nuova entità
	pthread_mutex_lock(&mutex_entityList);
	thisEntity.source = SUICIDE;	//Informa il mainLoop che si sta per terminare, in modo tale da cancellare la propria registrazione nella entityList
	pthread_mutex_unlock(&mutex_entityList);
	
	insertBuffer(mainBuffer, &counter_mainBuffer, &mutex_mainBuffer, &semaphore_mainBuffer, thisEntity);

	
	///Crea una nave di prossimo livello
	if (thisEntity.level < ENEMY_LEVEL3){
		thisEntity.level++;
		thisEntity.source = thisEntity.parent;	//Riprtistina l'id
		pthread_create (&thread_enemyNext, NULL, &enemyInit, &thisEntity);
	}
	
	/// Morte del thread
	return;
	
	
}
/***********************************************************************
 * Nome: timerFire
 * Parametri: 
 * 		arg: id di chi ha invocato timerFire (necessita casting)
 * Gestisce gli intervalli tra gli spari
 * ********************************************************************/
void* timerFire(void* arg){
	
	EntityParams fireSignal;
	int i;
	int id=(int)arg;
	fireSignal.source = FIRE;
	

	switch (id){
		case ID_STARSHIP:
			starshipFirePermission = DENIED ;
			usleep(FIRE_DELAY_STARSHIP);
			starshipFirePermission = ALLOWED;
			break;
	
		default:
			while(1){
				sleep(FIRE_DELAY_ENEMY+(id*2));
				insertBuffer(enemyTimerBuffer[id-1], &counter_enemyTimerBuffer[id-1], &mutex_enemyTimerBuffer[id-1], &semaphore_enemyTimerBuffer[id-1], fireSignal);
			}		
			break;
	}

}
/***********************************************************************
 * Nome: timerMove
 * Parametri:
 * 		Nessuno (ma utilizza una variabile globale)
 * Incrementa una variabile globale a intervalli regolari
 * ********************************************************************/
void* timerMove(void* arg){
	global_y_position = 0;
	while(1){
		sleep(MOVE_Y_DELAY);
		global_y_position++;
	}
}
/***********************************************************************
 * Nome: missilesInit
 * Parametri:
 * 		Nessuno
 * Crea, tramite fork(), i due missili e un timerFire, che serve a 
 * impedire la generazione di altri missili fino alla propria morte.
 * ********************************************************************/
void missilesInit(){
	
	int id_left = ID_MISSILELEFT;
	int id_right = ID_MISSILERIGHT;
	
	pthread_t thread_missile_left;
	pthread_t thread_missile_right;

			
	pthread_create (&thread_missile_left, NULL, &missileLoop, (void *)id_left);
	pthread_create (&thread_missile_right, NULL, &missileLoop, (void *)id_right);
	
	return;
}
/**********************************************************************
 * Nome: bombInit
 * Parametri:
 * 		id: il numero dell'enemy che ha creato la bomba
 * La funzione crea una bomba, tramite pthread_create
 * *******************************************************************/
void bombInit(int id){
	
	pthread_t thread_bomb;
	pthread_create(&thread_bomb, NULL, &bombLoop, (void*)id);
	
	return;
}
/***********************************************************************
 * Nome: bombLoop
 * Parametri:
 * 		arg: il numero dell'enemy che ha creato la bomba (necessita casting)
 * La funzione bombLoop, si occupa di posizionare la bomba a seconda del 
 * livello dell'enemy, nonchè dello spostamento e della terminazione della 
 * bomba stessa
 * ********************************************************************/
void* bombLoop(void* arg){
	int id=(int)arg;
	EntityParams bomb;
	
	pthread_mutex_lock(&mutex_entityList);
	bomb = entityList[id];
	pthread_mutex_unlock(&mutex_entityList);
	
	bomb.source = ID_BOMB;
	bomb.tr = pthread_self();
	bomb.y = bomb.y + bomb.level;

	if (bomb.level==ENEMY_LEVEL1) bomb.x++;
	if (bomb.level==ENEMY_LEVEL2) bomb.x++;
	if (bomb.level==ENEMY_LEVEL3) bomb.x += 2;
		
	//~ mvprintw(M+id, MAXX, "Bomb: x=%2d, y=%2d", bomb.x, bomb.y); refresh();
	
	while (bomb.y < MAXY){
		insertBuffer(mainBuffer, &counter_mainBuffer, &mutex_mainBuffer, &semaphore_mainBuffer, bomb);
		bomb.y++;
		usleep(BOMB_DELAY);
	}
	
	return;
}
/***********************************************************************
 * Nome: missileLoop
 * Parametri:
 * 		arg: tipo di missile (left o right) (necessita casting)
 * Si occupa di determinare le coordiante iniziali e i successivi 
 * spostamenti dei missili.
 * ********************************************************************/
void* missileLoop(void* arg){
	
	int id=(int)arg;
	EntityParams missile;
	int ciclo=0;	
	//~ mvprintw(20, MAXX+0, "Entrato nel missileLoop"); //DEBUG
	missile.tr = pthread_self();
	missile.level = MISSILE_LEVEL;
	missile.source=id;

	switch (missile.source)	{
		case ID_MISSILELEFT:

			missile.y = starship_global.y-1;
			missile.x = starship_global.x+2;

			insertBuffer(mainBuffer, &counter_mainBuffer, &mutex_mainBuffer, &semaphore_mainBuffer, missile);

			do{ //Loop vitale
				//~ mvprintw(MAXY-1, MAXX+0, "Missile SX sparato, x=%2d y=%2d", missile.x, missile.y); refresh(); //DEBUG
				missile.y--;
				missile.x--;
				insertBuffer(mainBuffer, &counter_mainBuffer, &mutex_mainBuffer, &semaphore_mainBuffer, missile);
				usleep(MISSILE_DELAY);
			}while((missile.y >= 0) && (missile.x>=0));
			break;
			
		case ID_MISSILERIGHT:


			missile.y = starship_global.y-1;
			missile.x = starship_global.x+3;

			insertBuffer(mainBuffer, &counter_mainBuffer, &mutex_mainBuffer, &semaphore_mainBuffer, missile);

			do{ //Loop vitale
				//~ mvprintw(MAXY-1, MAXX+0, "Missile DX sparato, x=%2d y=%2d", missile.x, missile.y); refresh(); //DEBUG
				missile.y--;
				missile.x++;
				insertBuffer(mainBuffer, &counter_mainBuffer, &mutex_mainBuffer, &semaphore_mainBuffer, missile);
				usleep(MISSILE_DELAY);
			}while((missile.y >= 0) && (missile.x<=MAXX-1));
			break;		
	}

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
	
	if (thisEntity.source == ID_BOMB){	//Controlla solo se colpisce il giocatore
		value = checkCoordinates(thisEntity, entityList[ID_STARSHIP]);

		//~ mvprintw(MAXY-2, 0, "checkCollision: value = %2d", value);  refresh(); //DEBUG
		if (value == 0) return value;
		else return -1;

	}
	
	for (i=0; i<(M+1); i++){
		if ((i != thisEntity.source) && (entityList[i].source != -1))
			value = checkCoordinates(thisEntity, entityList[i]);
		if (value != -1) return value;		//Si blocca alla prima collisione rilevata
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
	
	//~ mvprintw(MAXY-2, 0, "thisEntitySizeX: %d, thisEntitySizeY: %d, otherEntitySizeX: %d, otherEntitySizey: %d", thisEntitySizeX, thisEntitySizeY, otherEntitySizeX, otherEntitySizeY); refresh(); //DEBUG
	

	for (j=0; j<thisEntitySizeY; j++){	//Cicla le righe di thisEntity
		for (k=0; k<thisEntitySizeX; k++){	//Cicla le colonne di thisEntity
			for (n=0; n<otherEntitySizeY; n++){		//Cicla le righe di otherEntity
				for (m=0; m<otherEntitySizeX; m++){		//Cicla le colonne di otherEntity
					if ((thisEntity.x+k == otherEntity.x+m) && (thisEntity.y+j == otherEntity.y+n)){
						//~ mvprintw(MAXY-1, 0, "checkCoordinates: Entity(%3d) ha colliso su Entity(%3d)", thisEntity.source, otherEntity.source); refresh(); //DEBUG
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
void removeEntityFromList(int entityNumber){
	entityList[entityNumber].source = -1;
	entityList[entityNumber].tr = -1;
	entityList[entityNumber].level = -1;
	entityList[entityNumber].x = -1;
	entityList[entityNumber].y = -1;
	entityList[entityNumber].parent = -1;
	entityList[entityNumber].direction = -1;
}
/***********************************************************************
 * Nome: epilogue
 * Parametri:
 * 		Nessuno
 * Stampa l'esito della partita
 * ********************************************************************/
void epilogue(){
	double punteggio = (score*3*1000)/5;
	
	erase();
	
	if (status == WIN)
		mvprintw(MAXY/2, MAXX/2-15, "Hai vinto! Il tuo punteggio è %6.2f", punteggio); 
		
	else mvprintw(MAXY/2, MAXX/2-15, "Hai perso! Il tuo punteggio è %6.2f", punteggio);
	
	pthread_mutex_lock(&mutex_curses);
	refresh();
	pthread_mutex_unlock(&mutex_curses);
	
	sleep(8);
}
