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

#define MOUSE_SENSITIVITY 0.5f

#define BSHIFT 			0
#define GSHIFT 			8
#define RSHIFT 			16
#define ASHIFT 			24

#define BMASK			0x000000FF
#define GMASK			0x0000FF00
#define RMASK			0x00FF0000
#define AMASK			0xFF000000

#define ZBUF_DEPTH 		1048576.f
#define ZBUF_MIN		INT_MIN

#define CLIP_TRI_IN 	3
#define CLIP_TRI_OUT 	6
#define CLIP_NEAR		0.1f

#define PI 				3.14159265358979323846f
#define DEG2RAD			(PI/180.f)

#define COLLISION_FALSE 0
#define COLLISION_TRUE	1
#define COLLISION_DONE  2
#define THIRD 			0.333333f
#define SMALLNR 		0.000001f

#define MODEL_CNT		64
#define TRIANGLE_CNT	16384

enum model_flags
{
	MODEL_FLAG_DRAW		= 1<<0,
	MODEL_FLAG_COLLIDE	= 1<<1,
	MODEL_FLAG_FREE		= 1<<2,
};

typedef struct vec2i vec2i;
typedef struct vec3i vec3i;
typedef struct vec3f vec3f;
typedef struct vec2f vec2f;
typedef struct vec4f vec4f;
typedef struct mat4f mat4f;
typedef struct model_raw model_raw;
typedef struct texture texture;

typedef struct intersection intersection;

typedef struct camera camera;

typedef struct render_triangle render_triangle;
typedef struct game_triangle game_triangle;
typedef struct model model;

typedef struct input_data input_data;
typedef struct render_data render_data;

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
typedef struct model_raw
{
	int vcnt;
	vec3f *vp;
	vec2f *vt;
	vec3f *vn;
	vec3f *vc;
	int fcnt;
	int *fm;
	vec3f *fn;
} model_raw;
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
	vec2f face;

	vec3f pos;
	vec3f target;
	vec3f up;

	mat4f view;
	mat4f proj;
} camera;

typedef struct render_triangle
{
    vec3f a;
    vec3f b;
    vec3f c;
    vec3f n;

    vec2f uva;
    vec2f uvb;
    vec2f uvc;

	vec3f cola;
	vec3f colb;
	vec3f colc;

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
    unsigned int flags;

    int rtricnt;
    render_triangle *rtri;

    int gtricnt;
    game_triangle *gtri;

    texture *tex;
    mat4f trans;
} model;

typedef struct input_data 
{
	int fd_kb;
	char keys[KEY_MAX/8+1];

	int fd_mouse;
	int mouseactivity;
	signed char mousedata[MOUSEBYTECNT];
	int mouseshow;
	int mousex;
	int mousey;
} input_data;
typedef struct render_data
{
	int fd;
	int tty;
	unsigned int *fb;

	unsigned int width;
	unsigned int height;
// TODO: move these into RENDER_MEM
	unsigned int *buf;
	int *zbuf;
	int *zbufmin;

	int frametime;

	int modelcnt;
	model *models;
	int tricnt;
	render_triangle *tris;
} render_data;

// render.c functions
void render_init();
void render_free();
void render_run();
void render_flush();
void render_px(int x, int y, unsigned int color);
int render_getz(int x, int y);
void render_setz(int x, int y, int z);
void render_px_safe(int x, int y, unsigned int color);
int render_getz_safe(int x, int y);
void render_setz_safe(int x, int y, int z);

void zbuf_to_tga(const char *filename);

void render_frametime_update();

void text_draw(int x, int y, const char *text, unsigned int color);

void camera_angle_from_target(camera *cam);
void camera_target_from_angle(camera *cam);

void line(vec2i a, vec2i b, unsigned int color);
void line_dot(vec2i a, vec2i b, unsigned int color);
void triangle_color(vec3i a, vec3i b, vec3i c, vec3f ca, vec3f cb, vec3f cc);
void triangle_tex(vec3i a, vec3i b, vec3i c, vec2f uva, vec2f uvb, vec2f uvc, float bright, texture t);

void rect(vec2i a, vec2i size, unsigned int color);

model_raw loadiqe(const char *filename);
void unload_model_raw(model_raw m);

render_triangle* render_load_tris(model_raw m);
game_triangle* game_load_tris(model_raw m);
model* load_model(model_raw gmodel, model_raw rmodel, texture *t);
model* dupe_model(model *m);

texture loadtga(const char *filename);
void unloadtex(texture t);
void drawtex(texture t);

void triangle_clip_viewport(vec3f *posin, vec2f *uvin, vec3f *posout, vec2f *uvout, int *cntout);
void triangle_clip_single(vec3f in1, vec3f in2, vec3f out, vec2f in1uv, vec2f in2uv, vec2f outuv, vec3f *posout, vec2f *uvout);
void triangle_clip_double(vec3f in, vec3f out1, vec3f out2, vec2f inuv, vec2f out1uv, vec2f out2uv, vec3f *posout, vec2f *uvout);
int triangle_clip_plane(vec3f p, vec3f n, render_triangle in, render_triangle *out0,render_triangle *out1);

unsigned int color_rgb(unsigned char r, unsigned char g, unsigned char b); 
unsigned int brighten(unsigned int c, float b);

void print_mat(mat4f m);
void print_vec3f(vec3f v);
void print_vec3i(vec3i v);
void print_vec4f(vec4f v);


// game.c functions
/*
void game_init();
void game_flush();
void game_run(player *p, model *m, model *sphere);
*/

void game_frametime_update();

void input_init();
void input_flush();
void input_free();
int input_key(int key);
int input_mouse_button(char button);
int input_mouse_relx();
int input_mouse_rely();
int input_mouse_absx();
int input_mouse_absy();

// math.c functions
vec3f vec3f_cross(vec3f a, vec3f b);
float vec3f_dot(vec3f a, vec3f b);
float vec3f_len(vec3f a);
float vec3f_len2(vec3f a);
int vec3i_dot(vec3i a, vec3i b);
float vec3f_dist(vec3f a, vec3f b);
float vec3f_dist2(vec3f a, vec3f b);
vec3f vec3f_add(vec3f a, vec3f b);
vec3i vec3i_add(vec3i a, vec3i b);
vec3f vec3f_sub(vec3f a, vec3f b);
vec3i vec3i_sub(vec3i a, vec3i b);
vec3f vec3f_mul(vec3f a, vec3f b);
vec3f vec3f_scale(vec3f a, float f);
vec3f vec3f_div(vec3f a, vec3f b);
vec3f vec3f_norm(vec3f a);
vec3f barycentric(vec3f a, vec3f b, vec3f c, vec3f p);
vec3f barycentric_i(vec3i a, vec3i b, vec3i c, vec3i p);
vec3f bary3carth(vec3i a, vec3i b, vec3i c, vec3f p);
vec2f bary2carth(vec2f a, vec2f b, vec2f c, vec3f p);
vec3f vec3f_trans(vec3f a, mat4f m);
vec3f vec3f_rotate(vec3f a, vec4f q);
vec3f vec_project_plane(vec3f p, vec3f o, vec3f n);
vec3f vec_project_line(vec3f p, vec3f a, vec3f b);
vec3f vec_project_segment(vec3f p, vec3f a, vec3f b);

float vec3f_dist_plane(vec3f p, vec3f n, vec3f a);

vec4f vec4f_mul(vec4f a, vec4f b);
vec4f vec4f_from_euler(float x, float y, float z);

float vec2f_dist(vec2f a, vec2f b);
vec2f vec2f_sub(vec2f a, vec2f b);
vec2f vec2f_add(vec2f a, vec2f b);
vec2f vec2f_div(vec2f a, vec2f b);
vec2f vec2f_div_f(vec2f a, float b);

void vec2i_swap(vec2i *a, vec2i *b);
void vec2f_swap(vec2f *a, vec2f *b);
void vec3i_swap(vec3i *a, vec3i *b);
void vec3f_swap(vec3f *a, vec3f *b);

vec3f vec3f_lerp(vec3f a, vec3f b, float amt);
vec3i vec3i_lerp(vec3i a, vec3i b, float amt);
vec2f vec2f_lerp(vec2f a, vec2f b, float amt);
vec4f vec4f_lerp(vec4f a, vec4f b, float amt);

float vec3f_lerp_plane(vec3f p, vec3f n, vec3f a, vec3f b);

vec3f vec3f_clamp(vec3f a, vec3f min, vec3f max);

float lerp(float a, float b, float amt);
float inv_lerp(float a, float b, float c);
float lerp_i(int a, int b, float amt);
float inv_lerp_i(int a, int b, int c);

float clamp_f(float a, float min, float max);
int clamp_i(int a, int min, int max);
float wrap_one_f(float a);


mat4f mat_identity();
mat4f mat_project(float fov, float aspect, float near, float far);
mat4f mat_mul(mat4f a, mat4f b);
mat4f mat_lookat(vec3f eye, vec3f center, vec3f up);
mat4f mat_transform(vec3f pos);
mat4f mat_rotate(vec3f rot);
mat4f mat_invert(mat4f a);
mat4f mat_transpose(mat4f a);
mat4f mat_rotation(vec3f angles);

// physics.c functions
int point_in_tri(vec3f p, vec3f a, vec3f b, vec3f c);
int ray_tri_intersect(vec3f o, vec3f dir, vec3f a, vec3f b, vec3f c, intersection *out);
int swept_tri_collision(vec3f pos, vec3f vel, vec3f a, vec3f b, vec3f c, vec3f n, collision *out);
int swept_sphere_collision(vec3f pos1, vec3f vel1, float radius1, vec3f pos2, float radius2, collision *out);

input_data INPUT_DATA;
render_data RENDER_DATA;
camera *CAMERA;

#endif // RENDER_H
