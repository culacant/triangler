#ifndef GAME_H
#define GAME_H

#include "render.h"

#define PROJECTILECNT	0xff

#define GRAVITY (vec3f){0.0f, 0.0f, 0.001f}
#define AIRCONTROL_FRAC		0.1f
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
	float face;
	vec3f r;
	unsigned int flags;
} player;
typedef struct projectile
{
	int ttl;
	vec3f pos;
	vec3f vel;
	model *m;
} projectile;
typedef struct projectiles
{
	int it;
	projectile arr[PROJECTILECNT];
} projectiles;


player player_init();
void player_free();

void player_vel_from_face(player *p);
void player_collide(player *p, model *m);

void projectiles_init();
void projectiles_free();
void projectiles_tick(int dt);

int projectile_add(projectile p);

projectiles PROJECTILES;

#endif 		// GAME_H
