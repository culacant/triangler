#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define KILOBYTE 1024
#define MEGABYTE 1024*KILOBYTE

#define POOL_SIZE 100*MEGABYTE

enum BLOCK_HDR_FLAGS
{
	FLAG_USED		= 1 << 0,
	FLAG_FREE		= 1 << 1,
	FLAG_END		= 1 << 2,
};

typedef struct block_hdr block_hdr;
typedef struct block_hdr
{
	unsigned int size;
	int flags;
} block_hdr;

char* init_mem(unsigned int size)
{
	char *out = malloc(size*sizeof(char)+sizeof(block_hdr));

	block_hdr *cur = (block_hdr*)out;
	cur->size = size*sizeof(char);
	cur->flags = FLAG_FREE | FLAG_END;

	return out;
}

void* mem_alloc(unsigned int size, char *mem)
{
	block_hdr *cur = NULL;
	block_hdr *new = (block_hdr*)mem;
	do
	{
		cur = new;
		if((cur->flags & FLAG_FREE) && (cur->size> size+sizeof(block_hdr) ))
		{
			int oldsize = cur->size;
			int oldflags = cur->flags;

			cur->flags = FLAG_USED;
			cur->size = size;

			new = cur+cur->size;
			new ->flags = oldflags;
			new->size = oldsize-size-sizeof(block_hdr);
			return cur;
		}
		new = cur+cur->size;
	}
	while(!(cur->flags & FLAG_END));
	return NULL;
}

void print_mem(char *mem)
{
	block_hdr *cur = NULL;
	block_hdr *new = (block_hdr*)mem;
	do
	{
		cur = new;
		for(int i=0;i<cur->size/10;i++)
		{
			if(cur->flags & FLAG_FREE)
				printf("F");
			else if(cur->flags & FLAG_USED)
				printf("U");
		}
		printf("\n");
		new = cur+cur->size;
	}
	while(!(cur->flags & FLAG_END));
}
void debug_mem(char *mem)
{
	block_hdr *cur = NULL;
	block_hdr *new = (block_hdr*)mem;
	do
	{
		cur = new;
		new = cur+cur->size;
	}
	while(!(cur->flags & FLAG_END));
}

int main()
{
	char *mem = init_mem(100);
	debug_mem(mem);

	mem_alloc(10, mem);
	debug_mem(mem);

	mem_alloc(20, mem);
	debug_mem(mem);

	mem_alloc(10, mem);
	debug_mem(mem);
	mem_alloc(30, mem);
	debug_mem(mem);

	debug_mem(mem);
	print_mem(mem);

	free(mem);
	return 0;
}

