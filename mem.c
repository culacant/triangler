#include "render.h"

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
