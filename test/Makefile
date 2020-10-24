CC = gcc 
#CFLAGS = -std=c99 -Wall -O3 -march=native 
CFLAGS = -std=c99 -Wall -ggdb  -D_POSIX_C_SOURCE
LDFLAGS = -lraylib -lX11 -lm
exe: 
	$(CC) arrays.c -o test.exe $(OBJ) $(CFLAGS) $(LDFLAGS)
exe4: 
	$(CC) raylib_clipping_2.c -o test.exe $(OBJ) $(CFLAGS) $(LDFLAGS)
exe3: 
	$(CC) raylib_tri.c -o test.exe $(OBJ) $(CFLAGS) $(LDFLAGS)
exe2: 
	$(CC) raylib_clipping.c -o test.exe $(OBJ) $(CFLAGS) $(LDFLAGS)
exe1: 
	$(CC) mem_array.c -o test.exe $(OBJ) $(CFLAGS) $(LDFLAGS)
