#include <stdio.h>
#include "../font.h"

int main()
{
for(int a=0; a<=128;a++)
{
	unsigned long long F = FONT[a];
	int i=0;
		F >>= 8;
	while (F) {
		if (F & 1)
			printf("*");
		else
			printf(" ");

		F >>= 1;
		i++;
		if(i%8 == 0)
			printf("\n");

	}
			printf("\n");
			printf("\n");
}
	return 0;
}
