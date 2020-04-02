#ifndef RENDER_H
#define RENDER_H

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <linux/kd.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <byteswap.h>
#include <errno.h>
#include <limits.h>
#include <time.h>

#define TTY_NAME 		"/dev/tty3"
#define FB_NAME 		"/dev/fb0"
#define KB_NAME 		"/dev/input/event4"
#define MOUSE_NAME 		"/dev/input/mice"

#define MOUSEBYTECNT 	3

#define BUTTON_LEFT 	0x1
#define BUTTON_RIGHT 	0x2
#define BUTTON_MIDDLE 	0x4

#define MOUSE_SENSITIVITY 0.001f

#define BSHIFT 			0
#define GSHIFT 			8
#define RSHIFT 			16
#define ASHIFT 			24

#define FOCUS_DIST 		50.0f
#define DEPTH 			1024
#define ZBUF_MIN		INT_MIN

#define CLIP_POINT_IN 	3
#define CLIP_POINT_OUT 	6
#define CLIP_NEAR		-0.1f

#define PI 				3.14159265358979323846

#define COLLISION_FALSE 0
#define COLLISION_TRUE	0
#define COLLISION_DONE  0
#define THIRD 			0.333333
#define SMALLNR 		0.000001f

typedef struct vec2i vec2i;
typedef struct vec3i vec3i;
typedef struct vec3f vec3f;
typedef struct vec2f vec2f;
typedef struct vec4f vec4f;
typedef struct mat4f mat4f;
typedef struct model model;
typedef struct texture texture;

typedef struct intersection intersection;

typedef struct camera camera;

typedef struct BUFF BUFF;
typedef struct INPUT INPUT;
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
typedef struct vec4f
{
	float x;
	float y;
	float z;
	float w;
} vec4f;
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
	mat4f trans;
} model;
typedef struct texture
{
	int width;
	int height;
	unsigned int *data;
} texture;

typedef struct intersection
{
	vec3f pos;
	vec3f normal;
	float distance;
} intersection;
typedef struct collision
{
	vec3f pos;
	vec3f vel;
	float distance2;
} collision;

typedef struct camera
{
	vec2f angle;

	vec3f pos;
	vec3f target;
	vec3f up;

	mat4f mv;
	mat4f proj;
	mat4f vp;
	mat4f fin;
} camera;

typedef struct BUFF
{
	unsigned int width;
	unsigned int height;
	unsigned int *fb;
	int tty;
	int fd;
	unsigned int *buf;
	int *zbuf;
	int *zbufmin;
} BUFF;
typedef struct INPUT 
{
	int fd_kb;
	char keys[KEY_MAX/8+1];

	int fd_mouse;
	int mouseactivity;
	signed char mousedata[MOUSEBYTECNT];
	int mouseshow;
	int mousex;
	int mousey;
} INPUT;
typedef struct MATS
{
	mat4f mv;
	mat4f proj;
	mat4f vp;
	mat4f fin;
} MATS;

// render.c functions
void buf_init();
void buf_free();
void buf_flush();
void buf_px(int x, int y, unsigned int color);
int buf_getz(int x, int y);
void buf_setz(int x, int y, int z);
void buf_px_safe(int x, int y, unsigned int color);
int buf_getz_safe(int x, int y);
void buf_setz_safe(int x, int y, int z);

void zbuf_to_tga(const char *filename);

void frametime_update();

void text_draw(int x, int y, const char *text, unsigned int color);

void input_init();
void input_flush();
void input_free();
int input_key(int key);
int input_mouse_button(char button);
int input_mouse_relx();
int input_mouse_rely();
int input_mouse_absx();
int input_mouse_absy();

void camera_update_mat(camera *cam);
void camera_angle_from_target(camera *cam);
void camera_target_from_angle(camera *cam);

camera camera_init();
void camera_free();

void line(vec2i a, vec2i b, unsigned int color);
void line_dot(vec2i a, vec2i b, unsigned int color);
void triangle_tex(vec3i a, vec3i b, vec3i c, vec2f uva, vec2f uvb, vec2f uvc, float bright, texture t);

void rect(vec2i a, vec2i size, unsigned int color);

model loadiqe(const char *filename);
void unloadmodel(model m);
void drawmodel_wire(model m, unsigned int color);
void drawmodel_tex(model m, texture t);

texture loadtga(const char *filename);
void unloadtex(texture t);
void drawtex(texture t);

void triangle_clip_viewport(vec3f *posin, vec2f *uvin, vec3f *posout, vec2f *uvout, int *cntout);
void triangle_clip_single(vec3f in1, vec3f in2, vec3f out, vec2f in1uv, vec2f in2uv, vec2f outuv, vec3f *posout, vec2f *uvout);
void triangle_clip_double(vec3f in, vec3f out1, vec3f out2, vec2f inuv, vec2f out1uv, vec2f out2uv, vec3f *posout, vec2f *uvout);

// math.c functions
vec3f vec_cross(vec3f a, vec3f b);
float vec_dot(vec3f a, vec3f b);
float vec_len(vec3f a);
float vec_len2(vec3f a);
int vec_dot_i(vec3i a, vec3i b);
float vec_dist(vec3f a, vec3f b);
vec3f vec_add(vec3f a, vec3f b);
vec3f vec_sub(vec3f a, vec3f b);
vec3i vec_sub_i(vec3i a, vec3i b);
vec3f vec_mul_f(vec3f a, float f);
vec3f vec_norm(vec3f a);
vec3f barycentric(vec3f a, vec3f b, vec3f c, vec3f p);
vec3f barycentric_i(vec3i a, vec3i b, vec3i c, vec3i p);
vec2f bary2carth(vec2f a, vec2f b, vec2f c, vec3f p);
vec3f vec_trans(vec3f a, mat4f m);
vec3f vec_project_plane(vec3f p, vec3f o, vec3f n);

void vec2i_swap(vec2i *a, vec2i *b);
void vec2f_swap(vec2f *a, vec2f *b);
void vec3i_swap(vec3i *a, vec3i *b);
void vec3f_swap(vec3f *a, vec3f *b);

vec3f vec3f_lerp(vec3f a, vec3f b, float amt);
vec3i vec3i_lerp(vec3i a, vec3i b, float amt);
vec2f vec2f_lerp(vec2f a, vec2f b, float amt);
vec4f vec4f_lerp(vec4f a, vec4f b, float amt);

float lerp(float a, float b, float amt);
float inv_lerp(float a, float b, float c);
float lerp_i(int a, int b, float amt);
float inv_lerp_i(int a, int b, int c);

int clamp_i(int a, int min, int max);
float wrap_one_f(float a);

mat4f mat_identity();
mat4f mat_viewport(int x, int y, int w, int h);
mat4f mat_mul(mat4f a, mat4f b);
mat4f mat_lookat(vec3f eye, vec3f center, vec3f up);
mat4f mat_transform(vec3f pos);

// physics.c functions
int point_in_tri(vec3f p, vec3f a, vec3f b, vec3f c);
int ray_tri_intersect(vec3f o, vec3f dir, vec3f a, vec3f b, vec3f c, intersection *out);
int swept_tri_collision(vec3f pos, float r, vec3f vel, vec3f a, vec3f b, vec3f c, vec3f n, collision *out);


// render.c functions
unsigned int color_rgb(unsigned int r, unsigned int g, unsigned int b); 
unsigned int brighten(unsigned int c, float b);

void print_mat(mat4f m);
void print_vec3f(vec3f v);
void print_vec3i(vec3i v);
void print_vec4f(vec4f v);

BUFF BUFFER;
INPUT INPUTS;
camera *CAMERA;
int FRAMETIME;

#endif // RENDER_H
