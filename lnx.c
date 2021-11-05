// render.h

/*
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <linux/kd.h>
#include <byteswap.h>		// <? still needed?
*/
#define KEY_MAX			5

#define TTY_NAME 		"/dev/tty3"
#define FB_NAME 		"/dev/fb0"
#define KB_NAME 		"/dev/input/event4"
#define MOUSE_NAME 		"/dev/input/mice"

#define MOUSEBYTECNT 	3

#define LNX_BUTTON_LEFT 	0x1
#define LNX_BUTTON_RIGHT 	0x2
#define LNX_BUTTON_MIDDLE 	0x4
#define KEY_W	1
#define KEY_S	1
#define KEY_A	1
#define KEY_D	1
#define KEY_SPACE 1
#define KEY_R	1
#define KEY_Q	1

#define KEYMAP_FORWARD		KEY_W
#define KEYMAP_BACKWARD		KEY_S
#define KEYMAP_STRAFELEFT	KEY_A
#define KEYMAP_STRAFERIGHT	KEY_D
#define KEYMAP_JUMP			KEY_SPACE
#define KEYMAP_FIRE			KEY_R
#define KEYMAP_QUIT			KEY_Q


/*
typedef struct input_data_lnx
{
	int fd_kb;								// <
	char keys[KEY_MAX/8+1];					// < ?

	int fd_mouse;							// <
	int mouseactivity;
	signed char mousedata[MOUSEBYTECNT];	// <
	int mouseshow;
	int mousex;
	int mousey;
} input_data_lnx;

typedef struct render_data_lnx
{
	int fd;									// <
	int tty;								// <
	unsigned int *fb;						// <

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
} render_data_lnx;
*/

// render.c

int lnx_fb_init();
void lnx_fb_free();
void lnx_fb_blit();

int lnx_input_init();
int lnx_input_free();
int lnx_input_flush();

int lnx_timer_get();

#define os_fb_init lnx_fb_init
#define os_fb_free lnx_fb_free
#define os_fb_blit lnx_fb_blit

#define os_input_init lnx_input_init
#define os_input_free lnx_input_free
#define os_input_flush lnx_input_flush

#define os_timer_get lnx_timer_get



int lnx_fb_init()
{
	struct fb_var_screeninfo sinfo;
	RENDER_DATA.fd = open(FB_NAME, O_RDWR);
	if (RENDER_DATA.fd <= 0)
	{
		printf("ERROR: cannot open framebuffer: %s\n", FB_NAME);
		return -1;
	}
	RENDER_DATA.tty = open(TTY_NAME, O_RDWR);
	if (RENDER_DATA.tty <= 0)
	{
		printf("ERROR: cannot open TTY: %s\n", TTY_NAME);
		return -1;
	}
	if (ioctl(RENDER_DATA.tty, KDSETMODE, KD_GRAPHICS) == -1)
	{
		printf("ERROR: cannot set graphics mode on TTY: %s\n", TTY_NAME);
		return -1;
	}
	if (ioctl(RENDER_DATA.fd, FBIOGET_VSCREENINFO, &sinfo) < 0)
	{
		printf("ERROR: get screen info failed: %s\n", strerror(errno));
		close(RENDER_DATA.fd);
		return -1;
	}

	RENDER_DATA.width = sinfo.xres;
	RENDER_DATA.height = sinfo.yres;

	RENDER_DATA.fb = mmap(NULL, 4 * RENDER_DATA.width * RENDER_DATA.height, PROT_READ | PROT_WRITE, MAP_SHARED, RENDER_DATA.fd, 0);
	if (RENDER_DATA.fb == NULL)
	{
		printf("ERROR: mmap framebuffer failed\n");
		close(RENDER_DATA.fd);
		return -1;
	}
	return 0;
}

void lnx_fb_free()
{
	munmap(RENDER_DATA.fb, 4*RENDER_DATA.width*RENDER_DATA.height);

	if(ioctl(RENDER_DATA.tty, KDSETMODE, KD_TEXT) == -1)
	{
		printf("WARNING: cannot set text mode on TTY: %s\n", TTY_NAME);
	}

	close(RENDER_DATA.tty);
	close(RENDER_DATA.fd);
}
void lnx_fb_blit()
{
	memcpy(RENDER_DATA.fb, RENDER_DATA.buf, sizeof(unsigned int)*RENDER_DATA.width*RENDER_DATA.height);
}
int lnx_input_init()
{
    INPUT_DATA.fd_kb = open(KB_NAME, O_RDONLY);
    INPUT_DATA.fd_mouse = open(MOUSE_NAME, O_RDONLY);
    fcntl(INPUT_DATA.fd_mouse, F_SETFL, O_NONBLOCK);

    if(INPUT_DATA.fd_kb <= 0)
    {    
        printf("ERROR: cannot open input: %s\n", KB_NAME);
        return;
    }    

    memset(INPUT_DATA.keys, 0, sizeof(INPUT_DATA.keys));

    if(INPUT_DATA.fd_mouse<= 0)
    {    
        printf("ERROR: cannot open input: %s\n", MOUSE_NAME);
        return;
    }
}
int lnx_input_free()
{
    close(INPUT_DATA.fd_kb);
    close(INPUT_DATA.fd_mouse);
}
int lnx_input_flush()
{
    memset(INPUT_DATA.mousedata, 0, sizeof(INPUT_DATA.mousedata));
    ioctl(INPUT_DATA.fd_kb, EVIOCGKEY(sizeof(INPUT_DATA.keys)), INPUT_DATA.keys);

    INPUT_DATA.mouseactivity = read(INPUT_DATA.fd_mouse, INPUT_DATA.mousedata, sizeof(INPUT_DATA.mousedata));
}
int lnx_timer_get()
{
	int out;
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	out = (int)(ts.tv_sec*1000 + ts.tv_nsec/1000000);
	return curtime;
}
/*
general:
	mmap
	munmap

render_init: fb_open
	RENDER_DATA.fd		// <? file descriptor
	RENDER_DATA.tty		// <? just tty file
	RENDER_DATA.width	// <
	RENDER_DATA.height	// <
	RENDER_DATA.fb		// < framebuffer to blit data to

render_free: fb_close
	RENDER_DATA.fd
	RENDER_DATA.tty

	RENDER_DATA.fb
	
render_flush
	RENDER_DATA.fb		// <? do more than just memcpy

render_frametime_update
	complete rewrite?
*/

// game.c
/*
general:
	key input, needs abstraction

game_frametime_update
	complete rewrite?

input_init
	INPUT_DATA.fd_kb		// <
	INPUT_DATA.fd_mouse		// <
	INPUT_DATA.keys			// < prolly complete rewrite, keep structure?
	INPUT_DATA.mousedata	// < prolly complete rewrite, keep structure?

input_free
	INPUT_DATA.fd_kb		// <
	INPUT_DATA.fd_mouse		// <

input_flush
	complete rewrite kb, get mouseactivity
*/
