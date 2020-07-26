#include "mem.h"

//debug
void print_mem(void *mem, unsigned int size)
{
	printf("\nMEMPRINT\n");
	block_hdr *cur = NULL;
	void *adr = mem;
	do
	{
		cur = adr;
		char *data = (char*)cur+sizeof(block_hdr);
// hdr
		printf("%p", cur); 
		printf("\t\t| ");
		if(cur->flags & FLAG_FREE)
			printf("F");
		else
			printf("U");
		if(cur->flags & FLAG_END)
			printf("E");
		printf("\t\t| ");
		printf("%i", cur->size);
		printf("\n");

		for(int i=0;i<cur->size;i++)
			printf("%c", data[i]);
		printf("\n");
		adr = (char*)cur+cur->size+sizeof(block_hdr);

	} while(!(cur->flags & FLAG_END));
}

//general
void *mem_init(unsigned int size)
{
	void *out = malloc(size+sizeof(block_hdr));
	block_hdr *cur = (block_hdr*)out;
	cur->flags = FLAG_FREE | FLAG_END;
	cur->size = size;
// fill with 0xBEEFBABE
	char *cdata = (char*)out;
	char data[] = "BEEFBABE";
	
	for(int i=sizeof(block_hdr);i<=size;i++)
		cdata[i] = data[i&7];

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
	} while(!(cur->flags & FLAG_END));
	printf("MEM_ALLOC ERROR SIZE %i MEM %p\n", size, mem);
	return NULL;
}
void mem_free(void *mem)
{
	block_hdr *cur = (block_hdr*)mem-1;
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

// specifics
void* malloc_render_tri(int cnt)
{
	void* addr = &RENDER_DATA.tris[RENDER_DATA.tricnt];
	RENDER_DATA.tricnt += cnt;
	return addr;
}
void* malloc_game_tri(int cnt)
{
	void* addr = &GAME_DATA.tris[GAME_DATA.tricnt];
	GAME_DATA.tricnt += cnt;
	return addr;
}
void* malloc_model(int cnt)
{
	void* addr = &GAME_DATA.models[GAME_DATA.modelcnt];
	GAME_DATA.modelcnt += cnt;
	return addr;
}
void* malloc_game_projectile(int cnt)
{
	void* addr = &GAME_DATA.projectiles[GAME_DATA.projectilecnt];
	GAME_DATA.projectilecnt+= cnt;
	return addr;
}
void free_projectile(projectile *p)
{
// TODO: free projectile and model mem
	p->ttl = -1;
	p->m->flags &= ~FLAG_DRAW;
	GAME_DATA.projectilecnt--;
}
