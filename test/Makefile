CC = gcc 
#CFLAGS = -std=c99 -Wall -O3 -march=native 
CFLAGS = -std=c99 -Wall -ggdb  -D_POSIX_C_SOURCE
LDFLAGS = -lraylib -lX11 -lm
exe: 
	$(CC) raylib_sweptsphere_2.c -o test.exe $(OBJ) $(CFLAGS) $(LDFLAGS)
