#ifndef GAME_H
#define GAME_H

#include "render.h"

#define BULLET_CNT			0x0f
#define MOB_CNT 			0x0f

#define IMPULSE				0.001f
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
enum mob_flags
{
	MOB_FLAG_FREE 	= 1<<0,
};

typedef struct player player;
typedef struct mob mob;
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
	vec3f radius;
	unsigned int flags;
	model *m;
} mob;
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
	int mobcnt;
	int mobit;
	mob *mobs;
	int bulletcnt;
	int bulletit;
	bullet *bullets;
} game_data;

void game_init();
void game_free();
void game_flush();
void game_run(player *p, model *sphere);

player player_init();
void player_free();

void player_update_vel(player *p);
void player_update_muzzle(player *p);
void player_collide_mobs(player *p);
void player_collide_models(player *p);
void player_fire(player *p);


mob* mob_add(vec3f pos, vec2f face, vec3f radius, model *m);
void mobs_tick(int dt);

bullet *bullet_add(vec3f pos, vec3f vel, float radius, model *m);
void bullets_tick(int dt);
int bullet_collide(bullet *p, model *m);


game_data GAME_DATA;

#endif 		// GAME_H
