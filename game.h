#ifndef GAME_H
#define GAME_H

#include "render.h"

typedef struct player
{
	vec3f pos;
	vec3f vel;
	float face;
	float r;
} player;

player player_init();
void player_free();

void player_vel_from_face(player *p);
void player_collide(player *p, model m);
#endif 		// GAME_H
