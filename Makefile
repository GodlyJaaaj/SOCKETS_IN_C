all:
	gcc server.c -g3 -lncurses -o server

all2:
	gcc client.c -g3 -lncurses -o client