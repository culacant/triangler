#ifndef GAME_H
#define GAME_H

#include "render.h"

#define BULLET_CNT			0x0f
#define MOB_CNT 			0x0f

#define IMPULSE				0.0001f
#define GRAVITY 			(vec3f){0.0f, 0.0f, -0.0001f}
#define FLOOR_Z_TRESHOLD	0.6f
#define JUMP_FRAC 			0.1f
#define DRAG_FRAC			0.5f
#define JUMP_HEIGHT			0.005f
#define ANTISTUCK_Z			0.000007f

enum player_flags
{
	FLAG_NONE		= 0,
	FLAG_AIR		= 1<<0,
	FLAG_GND		= 1<<1,
};

typedef struct player player;
typedef struct mob mob;
typedef struct mobs mobs;
typedef struct bullet bullet;

typedef struct game_data game_data;

typedef struct player
{
	vec3f pos;
	vec3f vel;
	vec3f impulse;
	vec2f face;
	vec3f r;
	unsigned int flags;
// weapon
	vec3f muzzle;
	vec3f muzzle_ofs;
} player;
typedef struct mob
{
	vec3f pos;
	vec2f face;
	vec3f r;
	unsigned int flags;
	model *m;
} mob;
typedef struct mobs
{
	int it;
	mob arr[MOB_CNT];
} mobs;
typedef struct bullet
{
	int ttl;
	vec3f pos;
	vec3f vel;
	float radius;
	model *m;
} bullet;

typedef struct game_data
{
    int frametime;

    int modelcnt;
    int modelit;
    model *models;
    int tricnt;
    game_triangle *tris;
	int bulletcnt;
	int bulletit;
	bullet *bullets;
} game_data;

void game_init();
void game_free();
void game_flush();
void game_run(player *p, model *m, model *sphere);

player player_init();
void player_free();

void player_update_vel(player *p);
void player_update_muzzle(player *p);
void player_collide(player *p);
void player_fire(player *p);

bullet *bullet_add(vec3f pos, vec3f vel, float radius, model *m);

void bullets_tick(int dt);
int bullet_collide(bullet *p, model *m);

void mobs_init();
void mobs_free();
int mob_add(mob m);

mobs MOBS;

game_data GAME_DATA;

#endif 		// GAME_H
