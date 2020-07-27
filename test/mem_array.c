#include <stdlib.h>
#include <stdio.h>
#include <string.h>

enum MEM_ARRAY_FLAGS
{
	FLAG_USED 		= 1 << 1,
	FLAG_FREE		= 1 << 2,
};

typedef struct mem_arr mem_arr;
typedef struct mem_arr
{
	unsigned int flags;
	int data;
} mem_arr;

unsigned int ARR_CNT = 0;
unsigned ARR_IT = 0;
#define ARR_MAX 10
mem_arr *ARR;

void print_arr(mem_arr *arr)
{
	for(int i=0;i<ARR_MAX;i++)
	{
		if(arr[i].flags & FLAG_USED)
			printf("%i \tU\t|\t %i\t|\n", i, arr[i].data);
		else
			printf("%i \tF\t|\t   \t|\n", i);
	}
	printf("\n\n");
}
void init_arr(mem_arr *arr, unsigned int max)
{
	for(int i=0;i<max;i++)
		arr[i].flags = FLAG_FREE;
}

void* alloc_arr(mem_arr *arr)
{
	int i = 0;
	while(i < ARR_MAX)
	{
		if(arr[ARR_IT].flags & FLAG_FREE)
		{
			arr[ARR_IT].flags = FLAG_USED;
			arr[ARR_IT].data = ARR_IT;
			ARR_CNT++;
			return &arr[ARR_IT];
			
		}
		ARR_IT++;
		if(ARR_IT > ARR_MAX)
		{
			ARR_IT = 0;
		}
		i++;
	}
	printf("MEM ERR\n");
	return 0;
}
void free_arr(mem_arr *arr)
{
	arr->flags = FLAG_FREE;
	ARR_CNT--;
}

int main()
{
	ARR = malloc(sizeof(mem_arr) *ARR_MAX);
	init_arr(ARR, ARR_MAX);

	mem_arr *a0 = alloc_arr(ARR);
	mem_arr *a1 = alloc_arr(ARR);
	mem_arr *a2 = alloc_arr(ARR);
	mem_arr *a3  = alloc_arr(ARR);
	mem_arr *a4  = alloc_arr(ARR);
	mem_arr *a5  = alloc_arr(ARR);
	mem_arr *a6  = alloc_arr(ARR);
	mem_arr *a7  = alloc_arr(ARR);
	mem_arr *a8  = alloc_arr(ARR);
	mem_arr *a9  = alloc_arr(ARR);

	free_arr(a5);
	print_arr(ARR);
	mem_arr *a10  = alloc_arr(ARR);
	print_arr(ARR);
	return 0;
}
