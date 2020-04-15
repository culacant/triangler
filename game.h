#ifndef GAME_H
#define GAME_H

#include "render.h"

#define PROJECTILECNT	0xff

typedef struct player player;
typedef struct projectile projectile;
typedef struct projectiles projectiles;

typedef struct player
{
	vec3f pos;
	vec3f vel;
	float face;
	vec3f r;
} player;
typedef struct projectile
{
	int ttl;
	vec3f pos;
	vec3f vel;
	model_raw m;
	texture *t;
} projectile;
typedef struct projectiles
{
	int it;
	projectile arr[PROJECTILECNT];
} projectiles;


player player_init();
void player_free();

void player_vel_from_face(player *p);
void player_collide(player *p, model m);

void projectiles_init();
void projectiles_free();
void projectiles_tick(int dt);

int projectile_add(projectile p);

projectiles PROJECTILES;

#endif 		// GAME_H
