#ifndef LNX_H
#define LNX_H

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <linux/kd.h>
#include <unistd.h>
#include <byteswap.h>

#define TTY_NAME               "/dev/tty3"
#define FB_NAME                "/dev/fb0"
#define KB_NAME                "/dev/input/event4"
#define MOUSE_NAME             "/dev/input/mice"

#define RSHIFT	16
#define GSHIFT	8
#define BSHIFT	0
#define ASHIFT	24

#define RMASK	0x000000FF
#define GMASK	0x0000FF00
#define BMASK	0x00FF0000
#define AMASK	0xFF000000

#define KEYMAP_FORWARD KEY_W
#define KEYMAP_BACKWARD KEY_S
#define KEYMAP_STRAFELEFT KEY_A
#define KEYMAP_STRAFERIGHT KEY_D
#define KEYMAP_JUMP KEY_SPACE
#define KEYMAP_FIRE KEY_R
#define KEYMAP_QUIT KEY_Q

#endif
