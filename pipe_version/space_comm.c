#include "space_comm.h"
 
/***********************************************************************
 * Nome: createPipe
 * Parametri:
 * 		filedes: descrittore di file
 * 		mode: modalità di inizializzazione della pipe
 * Crea le pipe
 * ********************************************************************/

int createPipe(int filedes[2], int mode){
	
	if (pipe(filedes)==-1) {
		perror("Errore nella creazione della pipe.");
		exit(1);
	}
	if (mode==UNBLOCK_R){
		if ( fcntl( filedes[0], F_SETFL, O_NDELAY) < 0 ) {
			perror("Error unblocking the pipe. ");
			return 0;
		}
	}
	if (mode==UNBLOCK_W){
		if ( fcntl( filedes[1], F_SETFL, O_NDELAY) < 0 ) {
			perror("Error unblocking the pipe. ");
			return 0;
		}
	}
	if (mode==UNBLOCK_RW){
		if ( fcntl( filedes[0], F_SETFL, O_NDELAY) < 0 ) {
			perror("Error unblocking the pipe. ");
			return 0;
		}
		if ( fcntl( filedes[1], F_SETFL, O_NDELAY) < 0 ) {
			perror("Error unblocking the pipe. ");
			return 0;
		}
	}
	return 1; //Va tutto bene
 }
 
 
/***********************************************************************
 * Nome: sendMessage
 * Parametri:
 * 		newMessage: il messaggio da inviare
 * 		writePipe: pipe in scrittura
 * Scrive un messaggio nella pipe per mezzo della write()
 * ********************************************************************/
void sendMessage(EntityParams newMessage, int writePipe){
	write(writePipe, &newMessage, sizeof(newMessage));
}

/***********************************************************************
 * Nome: sendMessage
 * Parametri:
 * 		readPipe: pipe in lettura
 * Riceve un messaggio nella pipe per mezzo della read()
 * ********************************************************************/
EntityParams receiveMessage(int readPipe){
	EntityParams theMessage;
	read(readPipe, &theMessage, sizeof(theMessage));
	return theMessage;
}
