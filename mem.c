#include "mem.h"

//debug
void print_mem(void *mem, char showdata, char *buf)
{
	sprintf(buf, "\nMEMPRINT\n");
	block_hdr *cur = NULL;
	void *adr = mem;
	do
	{
		cur = adr;
		char *data = (char*)cur+sizeof(block_hdr);
// hdr
		sprintf(buf, "%s%p  |  ", buf, cur); 
		if(cur->flags & FLAG_FREE)
			sprintf(buf, "%sF",buf);
		else
			sprintf(buf, "%sU",buf);
		if(cur->flags & FLAG_END)
			sprintf(buf, "%sE",buf);
		sprintf(buf, "%s  |  %i", buf, cur->size);
		sprintf(buf, "%s\n", buf);
		if(showdata)
		{
			for(int i=0;i<cur->size;i++)
				sprintf(buf, "%s%c", buf, data[i]);
			sprintf(buf, "%s\n",buf);
		}
		adr = (char*)cur+cur->size+sizeof(block_hdr);

	} while(!(cur->flags & FLAG_END));
}
void print_models(char *buf)
{
	sprintf(buf, "\nMODELS\n");
	for(int i=0;i<MODEL_CNT;i++)
	{
		sprintf(buf, "%s model %i: ", buf, i);
		model *cur = &GAME_DATA.models[i];
		if(cur->flags & MODEL_FLAG_DRAW)
			sprintf(buf, "%sD",buf);
		if(cur->flags & MODEL_FLAG_COLLIDE)
			sprintf(buf, "%sC", buf);
		if(cur->flags & MODEL_FLAG_FREE)
			sprintf(buf, "%sF", buf);
		sprintf(buf, "%s\n",buf);
	}
}
void print_bullets(char *buf)
{
	sprintf(buf, "\nBULLETS\n");
	for(int i=0;i<BULLET_CNT;i++)
	{
		sprintf(buf, "%s bullet %i: ", buf, i);
		bullet *cur = &GAME_DATA.bullets[i];
		sprintf(buf, "%sttl: %i model:",buf, cur->ttl);
		if(cur->m)
		{
			if(cur->m->flags & MODEL_FLAG_DRAW)
				sprintf(buf, "%sD", buf);
			if(cur->m->flags & MODEL_FLAG_COLLIDE)
				sprintf(buf, "%sC", buf);
			if(cur->m->flags & MODEL_FLAG_FREE)
				sprintf(buf, "%sF", buf);
		}
		else
				sprintf(buf, "%sNONE", buf);
		sprintf(buf, "%s\n",buf);
	}
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
	int i = 0;
	while(i < MODEL_CNT)
	{
		if(GAME_DATA.models[GAME_DATA.modelit].flags & MODEL_FLAG_FREE)
		{
			GAME_DATA.models[GAME_DATA.modelit].flags = 0;
			GAME_DATA.modelcnt++;
			return &GAME_DATA.models[GAME_DATA.modelit];
			
		}
		GAME_DATA.modelit++;
		if(GAME_DATA.modelit >= MODEL_CNT)
			GAME_DATA.modelit = 0;
		i++;

	}
	printf("MODELS FULL\n");
	return 0;
}
void free_model(model *m)
{
	m->flags = MODEL_FLAG_FREE;
	GAME_DATA.modelcnt --;
}

void* malloc_game_mob(int cnt)
{
	int i = 0;
	while(i < MOB_CNT)
	{
		if(GAME_DATA.mobs[GAME_DATA.mobit].flags & MOB_FLAG_FREE)
		{
			GAME_DATA.mobcnt++;
			return &GAME_DATA.mobs[GAME_DATA.mobit];
			
		}
		GAME_DATA.mobit++;
		if(GAME_DATA.mobit >= MOB_CNT)
			GAME_DATA.mobit= 0;
		i++;

	}
	printf("MOBS FULL\n");
	return 0;

}
void free_game_mob(mob *p)
{
	p->flags = MOB_FLAG_FREE;
	free_model(p->m);
	p->m = NULL;
	GAME_DATA.mobcnt--;
}
void* malloc_game_bullet(int cnt)
{
	int i = 0;
	while(i < BULLET_CNT)
	{
		if(GAME_DATA.bullets[GAME_DATA.bulletit].ttl < 0)
		{
			GAME_DATA.bulletcnt++;
			return &GAME_DATA.bullets[GAME_DATA.bulletit];
			
		}
		GAME_DATA.bulletit++;
		if(GAME_DATA.bulletit >= BULLET_CNT)
			GAME_DATA.bulletit = 0;
		i++;

	}
	printf("BULLETS FULL\n");
	return 0;

}
void free_game_bullet(bullet *p)
{
	p->ttl = -1;
	free_model(p->m);
	p->m = NULL;
	GAME_DATA.bulletcnt--;
}
