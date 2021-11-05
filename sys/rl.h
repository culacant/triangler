#ifndef RL_H
#define RL_H

#include "raylib.h"

#define KEY_MAX 128

#define RSHIFT	0
#define GSHIFT	8
#define BSHIFT	16
#define ASHIFT	24

#define RMASK	0x00FF0000
#define GMASK	0x0000FF00
#define BMASK	0x000000FF
#define AMASK	0xFF000000

#define KEYMAP_FORWARD KEY_W
#define KEYMAP_BACKWARD KEY_S
#define KEYMAP_STRAFELEFT KEY_A
#define KEYMAP_STRAFERIGHT KEY_D
#define KEYMAP_JUMP KEY_SPACE
#define KEYMAP_FIRE KEY_R
#define KEYMAP_QUIT KEY_Q

#endif
