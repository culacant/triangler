#ifndef GAME_H
#define GAME_H

#include "render.h"

#define PROJECTILECNT	0xff

#define GRAVITY (vec3f){0.0f, 0.0f, 0.001f}
#define FLOOR_Z_TRESHOLD	0.6f
#define JUMP_FRAC 			0.1f
#define DRAG_FRAC			0.5f
#define JUMP_HEIGHT			0.005f

enum player_flags
{
	FLAG_NONE		= 0,
	FLAG_AIR		= 1<<0,
	FLAG_GND		= 1<<1,
};

typedef struct player player;
typedef struct projectile projectile;
typedef struct projectiles projectiles;

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
typedef struct projectile
{
	int ttl;
	vec3f pos;
	vec3f vel;
	float radius;
	model *m;
} projectile;
typedef struct projectiles
{
	int it;
	projectile arr[PROJECTILECNT];
} projectiles;


player player_init();
void player_free();

void player_update_vel(player *p);
void player_update_muzzle(player *p);
void player_collide(player *p, model *m);
void player_fire(player *p);

void projectiles_init();
void projectiles_free();
void projectiles_tick(int dt, model *m);
int projectile_collide(projectile *p, model *m);

int projectile_add(projectile p);

projectiles PROJECTILES;

#endif 		// GAME_H
