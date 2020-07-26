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
void print_mem(void *mem, unsigned int size);
//general
void* mem_init(unsigned int size);
void* mem_alloc(unsigned int size, void *mem);
void mem_free(void *mem);

// specifics
void* malloc_render_tri(int cnt);
void* malloc_game_tri(int cnt);
void* malloc_model(int cnt);
void* malloc_projectile(int cnt);
void free_projectile(projectile *p);

#endif // MEM_H