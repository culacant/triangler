#include "lnx.h"
#include "render.h"


int os_fb_init()
{
       struct fb_var_screeninfo sinfo;
       RENDER_DATA.fd = open(FB_NAME, O_RDWR);
       if(RENDER_DATA.fd <= 0)
       {
               printf("ERROR: cannot open framebuffer: %s\n", FB_NAME);
               return;
       }
       RENDER_DATA.tty = open(TTY_NAME,O_RDWR);
       if(RENDER_DATA.tty <= 0)
       {
               printf("ERROR: cannot open TTY: %s\n", TTY_NAME);
               return;
       }
       /*
       if(ioctl(RENDER_DATA.tty, KDSETMODE, KD_GRAPHICS) == -1)
       {
               printf("ERROR: cannot set graphics mode on TTY: %s\n", TTY_NAME);
               return;
       }
       */
       if(ioctl(RENDER_DATA.fd, FBIOGET_VSCREENINFO, &sinfo) < 0)
       {
               printf("ERROR: get screen info failed: %s\n", strerror(errno));
               close(RENDER_DATA.fd);
               return;
       }

       RENDER_DATA.width = sinfo.xres;
       RENDER_DATA.height = sinfo.yres;

       RENDER_DATA.fb = mmap(NULL, 4*RENDER_DATA.width*RENDER_DATA.height, PROT_READ | PROT_WRITE, MAP_SHARED, RENDER_DATA.fd, 0);
       if(RENDER_DATA.fb == NULL)
       {
               printf("ERROR: mmap framebuffer failed\n");
               close(RENDER_DATA.fd);
               return;
       }

}

void os_fb_blit()
{
	memcpy(RENDER_DATA.fb, RENDER_DATA.buf, sizeof(unsigned int)*RENDER_DATA.width*RENDER_DATA.height);
}

void os_fb_free()
{
       munmap(RENDER_DATA.fb, 4*RENDER_DATA.width*RENDER_DATA.height);

       if(ioctl(RENDER_DATA.tty, KDSETMODE, KD_TEXT) == -1)
       {
               printf("WARNING: cannot set text mode on TTY: %s\n", TTY_NAME);
       }

       close(RENDER_DATA.tty);
       close(RENDER_DATA.fd);
	
}

int os_input_init()
{
    INPUT_DATA.fd_kb = open(KB_NAME, O_RDONLY);
    INPUT_DATA.fd_mouse = open(MOUSE_NAME, O_RDONLY);
    fcntl(INPUT_DATA.fd_mouse, F_SETFL, O_NONBLOCK);

    if(INPUT_DATA.fd_kb <= 0)
    {
        printf("ERROR: cannot open input: %s\n", KB_NAME);
        return -1;
    }

    memset(INPUT_DATA.keys, 0, sizeof(INPUT_DATA.keys));

    if(INPUT_DATA.fd_mouse<= 0)
    {
        printf("ERROR: cannot open input: %s\n", MOUSE_NAME);
        return -1;
    }
	return 0;
}
int os_input_free()
{
	close(INPUT_DATA.fd_kb);
	close(INPUT_DATA.fd_mouse);
}
int os_input_flush()
{
    memset(INPUT_DATA.mousedata, 0, sizeof(INPUT_DATA.mousedata));
    ioctl(INPUT_DATA.fd_kb, EVIOCGKEY(sizeof(INPUT_DATA.keys)), INPUT_DATA.keys);

    INPUT_DATA.mouseactivity = read(INPUT_DATA.fd_mouse, INPUT_DATA.mousedata, sizeof(INPUT_DATA.mousedata));
	return 0;
}

int os_timer_get()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int)(ts.tv_sec*1000 + ts.tv_nsec/1000000);
}
