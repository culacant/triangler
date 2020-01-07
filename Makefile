CC = gcc 
#CFLAGS = -std=c99 -Wall -O3 -march=native 
CFLAGS = -std=c99 -Wall -ggdb 
LDFLAGS = -lm
OBJ = obj/render.o obj/math.o
exe: render math
	$(CC) main.c -o render.exe $(OBJ) $(CFLAGS) $(LDFLAGS)
render: render.c
	$(CC) -c render.c -o obj/render.o $(CFLAGS)
math: math.c
	$(CC) -c math.c -o obj/math.o $(CFLAGS)
