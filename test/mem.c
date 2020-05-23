#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define KILOBYTE 1024
#define MEGABYTE 1024*KILOBYTE

#define POOL_SIZE 100*MEGABYTE

enum BLOCK_HDR_FLAGS
{
	FLAG_FREE		= 1 << 1,
	FLAG_END		= 1 << 2,
};

typedef struct block_hdr block_hdr;
typedef struct block_hdr
{
	unsigned int flags;
	unsigned int size;
} block_hdr;

void print_hdr(block_hdr *h)
{
	printf("%p", h);
	printf("\t\t| ");
	if(h->flags & FLAG_FREE)
		printf("F");
	else
		printf("U");
	if(h->flags & FLAG_END)
		printf("E");
	printf("\t\t| ");
	printf("%i", h->size);
	printf("\n");
}
void print_mem(void *mem, unsigned int size)
{
	printf("\n\n");
	block_hdr *cur = NULL;
	void *adr = mem;
	do
	{
		cur = adr;
		char *data = (char*)cur+sizeof(block_hdr);

		print_hdr(cur);

		for(int i=0;i<cur->size;i++)
		{
			printf("%c", data[i]);
		}
		printf("\n");

		adr = (char*)cur+cur->size+sizeof(block_hdr);;
	}
	while(!(cur->flags & FLAG_END));
}

void* init_mem(unsigned int size)
{
	void *out = malloc(size+sizeof(block_hdr));

	block_hdr *cur = (block_hdr*)out;
	cur->flags = FLAG_FREE | FLAG_END;
	cur->size = size;

// fill with 0xDEADBEEF
	char *cdata = (char*)out;
	char data[] = "DEADBEEF";
	for(int i=sizeof(block_hdr);i<=size;i++)
	{
		cdata[i] = data[i&7];
	}

	return out;
}

void* mem_alloc(unsigned int size, void *mem)
{
	block_hdr *cur = NULL;
	block_hdr *new = mem;
	void *adr;
	do
	{
		cur = new;
		if(cur->flags & FLAG_FREE)
		{
			if(cur->size >= size+sizeof(block_hdr))
			{
				adr = (void*)cur+(size+sizeof(block_hdr));

				new = adr;
				new->flags = cur->flags;
				new->size = cur->size-(size+sizeof(block_hdr));

				cur->flags = 0;
				cur->size = size;

				adr=cur;
				return adr+sizeof(block_hdr);
			}
		}
		adr = (void*)cur+(cur->size+sizeof(block_hdr));
		new = adr;
	}
	while(!(cur->flags & FLAG_END));
	printf("DONE\n");
	return NULL;
}
void mem_free(void *mem)
{
	block_hdr *cur = (block_hdr *) mem-1;
	block_hdr *next;
	void *adr = (void*)cur;

	cur->flags |= FLAG_FREE;

	adr += cur->size+sizeof(block_hdr);
	next = adr;
	if(next->flags & FLAG_FREE)
	{
		cur->flags = next->flags;
		cur->size += next->size+sizeof(block_hdr);
	}
}
void mem_defrag(void *mem)
{
}

void debug_mem(void *mem)
{
	block_hdr *cur = NULL;
	void *adr = mem;
	do
	{
		cur = adr;
		adr = (void*)cur+cur->size+sizeof(block_hdr);
	}
	while(!(cur->flags & FLAG_END));
}

int main()
{
	int *mem = init_mem(100);

	char* data1 = mem_alloc(sizeof(char)*10, mem);
	char* data2 = mem_alloc(sizeof(char)*10, mem);
	int* data3 = mem_alloc(sizeof(int)*10, mem);

	for(int i=0;i<10;i++)
		data1[i] = 'a';
	for(int i=0;i<10;i++)
		data2[i] = 'b';
	/*
	for(int i=0;i<10;i++)
		data3[i] = 'c';
	*/

	print_mem(mem, 100);

	mem_free(data2);

	print_mem(mem, 100);

	free(mem);
	return 0;
}
