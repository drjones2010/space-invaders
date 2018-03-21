#include "space_game.h"
#include "space_comm.h"


/***********************************************************************
 * Nome: insertBuffer
 * Parametri:
 * 		buffer[]: il buffer su cui scrivere
 * 		counter: puntatore al contatore del buffer
 * 		mutex: puntatore al mutex del buffer
 * 		semaphore: puntatore al semaphore del buffer
 * 		message: il messaggio da inserire nel buffer
 * Inserisce un elemento nel buffer e dispone dei controlli di sicurezza
 * per l'accesso alla sezione critica
 * ********************************************************************/
void insertBuffer(EntityParams buffer[], int* counter, pthread_mutex_t* mutex, sem_t* semaphore, EntityParams message){
	//Sezione ingresso
	pthread_mutex_lock(mutex);	//Indirizzo
	/** SEZIONE CRITICA DEL BUFFER **/
	if(*counter < DIM_BUFFER){
		buffer[*counter] = message;
		(*counter)++;
		sem_post(semaphore); // incrementa il semaforo
	}
	/******************************/
	//Sezione uscita
	pthread_mutex_unlock(mutex);	
}


/***********************************************************************
 * Nome: removeBuffer
 * Parametri:
 * 		buffer[]: il buffer da cui leggere
 * 		counter: puntatore al contatore del buffer
 * 		mutex: puntatore al mutex del buffer
 * 		semaphore: puntatore al semaphore del buffer
 * 		message: il messaggio da inserire nel buffer
 * Rimuove un elemento dal buffer e dispone dei controlli di sicurezza
 * per l'accesso alla sezione critica
 * ********************************************************************/
EntityParams removeBuffer(EntityParams buffer[], int* counter, pthread_mutex_t* mutex, sem_t* semaphore, int mode){
	EntityParams message;

	//Ingresso nella sezione critica
	int value;
	if (mode == WAIT)
		value = sem_wait(semaphore); // se non ci sono job, li aspetta
	if (mode == TRYWAIT)
		value = sem_trywait(semaphore); // se non ci sono job, non li aspetta ma non esce
		
	if (value == 0){
		pthread_mutex_lock(mutex);
		/**SEZIONE CRITICA DEL BUFFER**/
			message = buffer[(*counter)-1];
			memset(&buffer[(*counter)-1], 0, sizeof(EntityParams)); //Nullizza la cella del buffer 
			(*counter)--; 
		/******************************/
		//Uscita dalla sezione critica
		pthread_mutex_unlock(mutex);
		return message;
	}
	message.source = NOMSG;
	return message;
}


