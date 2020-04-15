#define _GNU_SOURCE
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sched.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct passing
{
	int status;
	int vals[4];
} passing;

static int child_func(void* arg) {
	passing *p = (passing *) arg;
	while(1)
	{
		kill(getpid(), SIGSTOP);
		printf("CHILD %i\n ", p->status);
		for(int i=0;i<4;i++)
			printf("%i, ", p->vals[i]);
		printf("\n");
	}
	return 0;
}

int main(int argc, char** argv) 
{
	// Allocate stack for child task.
	const int STACK_SIZE = 65536;
	char* stack = malloc(STACK_SIZE);
	if (!stack) 
	{
		perror("malloc");
		exit(1);
	}

	passing * p = calloc(sizeof(passing), 1);
	int pid = clone(child_func, stack + STACK_SIZE, CLONE_VM | SIGCHLD, p);
	if(pid == -1) 
	{
		perror("clone");
		exit(1);
	}
	printf("loopin\n");

	int x = 0;
	int y = 0;
	while(1)
	{
		x++;
		x = x%4;
		y++;
		p->vals[x] = y;
		printf("PARENT %i\n ", p->status);
		for(int i=0;i<4;i++)
			printf("%i, ", p->vals[i]);
		printf("\n");

		kill(pid, SIGCONT);
//		printf("PID %i\n", pid);
	}

	return 0;
}
