srcname=spaceinvaders
binfile=$(srcname).o
$(binfile): space_main.c space_game.c space_game.h space_comm.c space_comm.h space_graphics.c space_graphics.h 
	gcc space_main.c space_game.c space_comm.c space_graphics.c -lncurses -lpthread -o $(binfile)
