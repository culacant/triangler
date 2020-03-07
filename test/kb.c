#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
        char key_b[KEY_MAX/8 + 1];

int is_key_pressed(int fd, int key)
{


        return !!(key_b[key/8] & (1<<(key % 8)));
}


int main(int argc, char **argv)
{
    int fd2;
	fd2 = open("/dev/input/event4", O_RDONLY);
	printf("KEY_MAX %i\n", KEY_MAX);
        memset(key_b, 0, sizeof(key_b));
    while (1)
    {
        ioctl(fd2, EVIOCGKEY(sizeof(key_b)), key_b);
		if(is_key_pressed(fd2, KEY_A))
			printf("KEY A,");
		if(is_key_pressed(fd2, KEY_B))
			printf("KEY B,");
		printf("\nsleep\n");
		sleep(1);

    }
}
