CC = gcc 
CFLAGS = -std=c99 -Wall -ggdb -O3
LDFLAGS = -lm
OBJ = obj/render.o
exe: render
	$(CC) main.c -o render.exe $(OBJ) $(CFLAGS) $(LDFLAGS)
render: render.c
	$(CC) -c render.c -o obj/render.o $(CFLAGS)
