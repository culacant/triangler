CC = gcc 
#CFLAGS = -std=c99 -Wall -O3 -march=native 
CFLAGS = -std=c99 -Wall -ggdb  -D_POSIX_C_SOURCE
LDFLAGS = -lraylib -lX11 -lm
exe: 
	$(CC) angles.c -o test.exe $(OBJ) $(CFLAGS) $(LDFLAGS)
exe1: 
	$(CC) mem_array.c -o test.exe $(OBJ) $(CFLAGS) $(LDFLAGS)
exe2: 
	$(CC) mem_fixedhdr.c -o test.exe $(OBJ) $(CFLAGS) $(LDFLAGS)
