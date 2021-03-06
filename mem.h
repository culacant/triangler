#ifndef MEM_H
#define MEM_H

#include "render.h"
#include "game.h"

enum BLOCK_HDR_FLAGS
{
	FLAG_FREE	= 1 << 1,
	FLAG_END	= 1 << 2,
};

typedef struct block_hdr block_hdr;

typedef struct block_hdr {
	unsigned int flags;
	unsigned int size;
} block_hdr;

// debug
void print_mem(void *mem, char showdata, char *buf);
void print_models(char *buf);
void print_bullets(char *buf);
//general
void* mem_init(unsigned int size);
void* mem_alloc(unsigned int size, void *mem);
void mem_free(void *mem);

// specifics
void* malloc_render_tri(int cnt);
void* malloc_game_tri(int cnt);
void* malloc_model(int cnt);
void free_model(model *m);

void* malloc_game_mob(int cnt);
void free_game_mob(mob*p);
void* malloc_game_bullet(int cnt);
void free_game_bullet(bullet *p);

#define GAME_MEM_SIZE 1024*1024	// 1MB
#define RENDER_MEM_SIZE 1024*1024*32	// 32MB

void *GAME_MEM;
void *RENDER_MEM;

#endif // MEM_H
