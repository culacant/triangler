#include <stdio.h>

int main()
{
	int a = 5;
	int b = 20;
	printf("a %i b %i\n", a, b);

	a = a ^ b;
	b = a ^ b;
	a = a ^ b;
	printf("a %i b %i\n", a, b);
}
