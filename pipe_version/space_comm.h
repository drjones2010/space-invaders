#ifndef SPACE_PIPE_H
#define SPACE_PIPE_H


/**Opzioni**/
#define BLOCK 0
#define UNBLOCK 1
#define UNBLOCK_R 2
#define UNBLOCK_W 3
#define UNBLOCK_RW 4

#include <fcntl.h> //Per utilizzare le pipe non bloccanti
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <curses.h>

typedef struct{
	pid_t pid;			//Pid del mittente
	int source;			//Identità del mittente (MACRO)
	int level;			//Livello di difficoltà (0 per l'astronave, 1:3 per i nemici)
	int x;				//Coordinate del mittente
	int y;
	int direction;
    int bouncing;
	int parent;
} EntityParams;

int createPipe(int[2], int);

void sendMessage(EntityParams, int);
EntityParams receiveMessage(int);

#endif

