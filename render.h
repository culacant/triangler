#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <byteswap.h>
#include <errno.h>

#define FB_NAME "/dev/fb0"

#define BSHIFT 0
#define GSHIFT 8
#define RSHIFT 16
#define ASHIFT 24

#define DEPTH 500.0f
#define ZBUF_MAX 999999.0f

#define NUMBUFF 2

typedef struct vec2i vec2i;
typedef struct vec3f vec3f;
typedef struct vec2f vec2f;
typedef struct mat4f mat4f;
typedef struct model model;
typedef struct texture texture;

typedef struct BUFF BUFF;
typedef struct MATS MATS;

typedef struct vec2i
{
	int x;
	int y;
} vec2i;
typedef struct vec3i
{
	int x;
	int y;
	int z;
} vec3i;
typedef struct vec3f
{
	float x;
	float y;
	float z;
} vec3f;
typedef struct vec2f
{
	float x;
	float y;
} vec2f;
typedef struct mat4f 
{
	float m0, m4, m8, m12;
	float m1, m5, m9, m13;
	float m2, m6, m10, m14;
	float m3, m7, m11, m15;
} mat4f;
typedef struct model
{
	int vcnt;
	vec3f *vp;
	vec2f *vt;
	vec3f *vn;
	int fcnt;
	int *fm;
	vec3f *fn;
} model;
typedef struct texture
{
	int width;
	int height;
	unsigned int *data;
} texture;

typedef struct BUFF
{
	unsigned int width;
	unsigned int height;
	unsigned int *buf;
	int fb;
	int curbuf;
	int bufcnt;
	unsigned int **buffers;
	float *zbuf;
	float *zbufmax;
} BUFF;
typedef struct MATS
{
	mat4f mv;
	mat4f proj;
	mat4f vp;
	mat4f fin;
} MATS;

void buf_init();
void buf_free();
void buf_flush();
void buf_px(int x, int y, unsigned int color);
float buf_getz(int x, int y);
void buf_setz(int x, int y, float z);

void update_mv(mat4f mv);
void update_proj(mat4f proj);
void update_vp(mat4f vp);

void line(vec2i a, vec2i b, unsigned int color);
void triangle_color(vec3f a, vec3f b, vec3f c, unsigned int color);
void triangle_tex(vec3f a, vec3f b, vec3f c, vec2f uva, vec2f uvb, vec2f uvc, float bright, texture t);
void rect(vec2i a, vec2i size, unsigned int color);

model loadiqe(const char *filename);
void unloadmodel(model m);
void drawmodel_wire(model m, unsigned int color);
void drawmodel(model m);
void drawmodel_tex(model m, texture t);

texture loadtga(const char *filename);
void unloadtex(texture t);
void drawtex(texture t);

vec3f vec_cross(vec3f a, vec3f b);
float vec_dot(vec3f a, vec3f b);
int vec_dot_i(vec3i a, vec3i b);
vec3f vec_add(vec3f a, vec3f b);
vec3f vec_sub(vec3f a, vec3f b);
vec3i vec_sub_i(vec3i a, vec3i b);
vec3f vec_norm(vec3f a);
vec3f barycentric(vec3f a, vec3f b, vec3f c, vec3f p);
vec3f barycentric_i(vec3i a, vec3i b, vec3i c, vec3i p);
vec2f bary2carth(vec2f a, vec2f b, vec2f c, vec3f p);
vec3f vec_trans(vec3f a, mat4f m);

mat4f identity();
mat4f viewport(int x, int y, int w, int h);
mat4f mat_mul(mat4f a, mat4f b);
mat4f mat_lookat(vec3f eye, vec3f center, vec3f up);

unsigned int color(unsigned int r, unsigned int g, unsigned int b); 
unsigned int brighten(unsigned int c, float b);
/*
float lerp(float min, float max, float v)
{
	return min + v * (max-min);
}
*/

BUFF BUFFER;
MATS MATRICES;
