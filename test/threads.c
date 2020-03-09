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

int status_2 = 20;

static int child_func(void* arg) {
	int *proc_status = (int *)arg;
	while(1)
	{
		if(*proc_status %2)
		{
			printf("CHILD %i, %i\n", *proc_status, status_2);
			kill(getpid(), SIGSTOP);
		}
	}
	return 0;
}

int main(int argc, char** argv) {
	// Allocate stack for child task.
	const int STACK_SIZE = 65536;
	char* stack = malloc(STACK_SIZE);
	if (!stack) {
		perror("malloc");
		exit(1);
	}

	int proc_status[1];
	proc_status[0] = 4;
	int pid = clone(child_func, stack + STACK_SIZE, CLONE_VM | SIGCHLD, proc_status);
	if(pid == -1) {
		perror("clone");
		exit(1);
	}
	printf("loopin\n");

	while(1)
	{
		printf("PARENT %i, %i\n", proc_status[0], status_2);
		proc_status[0]++;
		status_2++;
		sleep(3);
		kill(pid, SIGCONT);
//		printf("PID %i\n", pid);
	}

	return 0;
}
