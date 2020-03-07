CC = gcc 
#CFLAGS = -std=c99 -Wall -O3 -march=native 
CFLAGS = -std=c99 -Wall -ggdb 
LDFLAGS = -lraylib -lX11 -lm
exe: 
	$(CC) raylib.c -o test.exe $(OBJ) $(CFLAGS) $(LDFLAGS)
