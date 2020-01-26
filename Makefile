CC = gcc 
#CFLAGS = -std=c99 -Wall -O3 -march=native -D_POSIX_SOURCE -D_GNU_SOURCE
# -D_* for time functions (#define _POSIX_C_SOURCE 199309L)
CFLAGS = -std=c99 -Wall -ggdb  -D_POSIX_SOURCE -D_GNU_SOURCE
LDFLAGS = -lm

exe: 
	$(CC) main.c -o render.exe $(CFLAGS) $(LDFLAGS)
