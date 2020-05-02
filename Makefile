CC = gcc 
# -D_* for time functions (#define _POSIX_C_SOURCE 199309L)
#CFLAGS = -std=c99 -ggdb -Wall -O3 -D_POSIX_SOURCE -D_GNU_SOURCE
# breaks valgrind
CFLAGS = -std=c99 -Wall -O3 -march=native -D_POSIX_SOURCE -D_GNU_SOURCE
CFLAGS_DEBUG = -std=c99 -Wall -ggdb  -D_POSIX_SOURCE -D_GNU_SOURCE
LDFLAGS = -lm

exe: 
	$(CC) final.c -o render.exe $(CFLAGS) $(LDFLAGS)
debug: 
	$(CC) final.c -o render.exe $(CFLAGS_DEBUG) $(LDFLAGS)
