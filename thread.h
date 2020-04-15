#ifndef THREAD_H
#define THREAD_H

#include "render.h"

#define TRIANGLE_CNT	4048
#define MODEL_CNT		32

typedef struct render_triangle render_triangle;
typedef struct game_triangle game_triangle;
typedef struct model model;
typedef struct render_buffers render_buffers;
typedef struct game_buffers game_buffers;

typedef struct render_triangle
{
	vec3f a;
	vec3f b;
	vec3f c;
	vec3f n;

	vec2f uva;
	vec2f uvb;
	vec2f uvc;

} render_triangle;
typedef struct game_triangle
{
	vec3f a;
	vec3f b;
	vec3f c;
	vec3f n;
} game_triangle;

typedef struct model 
{
	int draw;

	int cnt_rtri;
	render_triangle *rtri;

	int cnt_gtri;
	game_triangle *gtri;

	texture *tex;
	mat4f trans;
} model;
typedef struct render_buffers 
{
	int mbuff_cnt;
	model mbuff[MODEL_CNT];
	int tbuff_cnt;
	render_triangle tbuff[TRIANGLE_CNT];
} render_buffers;
typedef struct game_buffers 
{
	int mbuff_cnt;
	model mbuff[MODEL_CNT];
	int tbuff_cnt;
	game_triangle tbuff[TRIANGLE_CNT];
} game_buffers;

render_triangle* rbuff_load_model(int vcnt, vec3f *vp, vec2f *vt, vec3f *vn, int fcnt, int *fn);
game_triangle* gbuff_load_model(int vcnt, vec3f *vp, vec3f *vn, int fcnt, int *fn);

model* buff_load_model(model_raw m, texture *t);

void render_rbuff();

// buffers
void* malloc_rbuff_tri(int cnt);
void* malloc_gbuff_tri(int cnt);
void* malloc_mbuff(int cnt);

render_buffers RENDER_BUFFERS;
game_buffers GAME_BUFFERS;

#endif		//THREAD_H
