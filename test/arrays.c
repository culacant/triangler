#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	srand(1);
	int incnt = 10;
	int in[10];
	int outcnt = 0;
	int out[20];

	printf("inarr:\n");
	for(int i=0;i<incnt;i++)
	{
		in[i] = i;
		printf("%x\n", in[i]);
	}


	for(int i=0;i<incnt;i++)
	{
		int cnt = rand()%3;
		printf("element %x split into %i\n", in[i], cnt);
		for(int j=0;j<cnt;j++)
		{
			int outval = (in[i]<<4) + j;
			out[outcnt+j] = outval;
		}
		outcnt += cnt;
	}
	printf("out 1st it:\n");
	for(int i=0;i<outcnt;i++)
	{
		printf("%x\n", out[i]);
	}
	incnt = outcnt;
	outcnt = 0;
	memcpy(in, out, sizeof(int)*incnt);





	for(int i=0;i<incnt;i++)
	{
		int cnt = rand()%3;
		printf("element %x split into %i\n", in[i], cnt);
		for(int j=0;j<cnt;j++)
		{
			int outval = (in[i]<<4) + j;
			out[outcnt+j] = outval;
		}
		outcnt += cnt;
	}
	printf("out 2nd it:\n");
	for(int i=0;i<outcnt;i++)
	{
		printf("%x\n", out[i]);
	}

	printf("outarr:\n");
	for(int i=0;i<outcnt;i++)
	{
		printf("%x\n", out[i]);
	}
	return 0;
}
