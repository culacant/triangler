#include <stdio.h>
#include <stdlib.h>

int main()
{
	char a[1024];
	char *b = malloc(sizeof(char)*1024);
	printf("%p %p\n", a, b);
	return 0;
}
