CC=	gcc
CFLAGS=	-W -Wall
OBJ=	simu
SRC=	events.c main.c parser.c regress.c util.c

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OBJ) -lm

clean:
	rm -rf $(OBJ) simu.dSYM
