#include "render.h"
#include "font.h"

void buf_init()
{
	struct fb_var_screeninfo sinfo;
	BUFFER.fd = open(FB_NAME, O_RDWR);
	if(BUFFER.fd <= 0)
	{
		printf("ERROR: cannot open framebuffer: %s\n", FB_NAME);
		return;
	}
	BUFFER.tty = open(TTY_NAME,O_RDWR);
	if(BUFFER.tty <= 0)
	{
		printf("ERROR: cannot open TTY: %s\n", TTY_NAME);
		return;
	}
// TODO: uncomment when things stop crashing
/*
	if(ioctl(BUFFER.tty, KDSETMODE, KD_GRAPHICS) == -1)
	{
		printf("ERROR: cannot set graphics mode on TTY: %s\n", TTY_NAME);
		return;
	}
*/
	if(ioctl(BUFFER.fd, FBIOGET_VSCREENINFO, &sinfo) < 0)
	{
		printf("ERROR: get screen info failed: %s\n", strerror(errno));
		close(BUFFER.fd);
		return;
	}

	BUFFER.width = sinfo.xres;
	BUFFER.height = sinfo.yres;

	BUFFER.buf = mmap(NULL, 4*BUFFER.width*BUFFER.height, PROT_READ | PROT_WRITE, MAP_SHARED, BUFFER.fd, 0);
	if(BUFFER.buf == NULL)
	{
		printf("ERROR: mmap framebuffer failed\n");
		close(BUFFER.fd);
		return;
	}

	BUFFER.bufcnt = NUMBUFF;
	BUFFER.curbuf = 0;
	BUFFER.buffers = malloc(sizeof(unsigned int *) * BUFFER.bufcnt);

	for(int i=0;i<BUFFER.bufcnt;i++)
		BUFFER.buffers[i] = calloc(BUFFER.width*BUFFER.height, sizeof(unsigned int));

	BUFFER.zbuf = malloc(sizeof(int) * BUFFER.width*BUFFER.height);
	BUFFER.zbufmin = malloc(sizeof(int) * BUFFER.width*BUFFER.height);

	for(int i=0;i<BUFFER.width*BUFFER.height;i++)
		BUFFER.zbufmin[i] = ZBUF_MIN;

	memcpy(BUFFER.zbuf, BUFFER.zbufmin, sizeof(int)*BUFFER.width*BUFFER.height);
	frametime_update();
}
void buf_free()
{
	for(int i=0;i<BUFFER.bufcnt;i++)
		free(BUFFER.buffers[i]);
	free(BUFFER.buffers);
	free(BUFFER.zbuf);
	free(BUFFER.zbufmin);
	munmap(BUFFER.buf,4*BUFFER.width*BUFFER.height);

	if(ioctl(BUFFER.tty, KDSETMODE, KD_TEXT) == -1)
	{
		printf("WARNING: cannot set text mode on TTY: %s\n", TTY_NAME);
	}

	close(BUFFER.tty);
	close(BUFFER.fd);
}
void buf_flush()
{
	memcpy(BUFFER.buf, BUFFER.buffers[BUFFER.curbuf], sizeof(unsigned int)*BUFFER.width*BUFFER.height);
	memset(BUFFER.buffers[BUFFER.curbuf], 0, sizeof(unsigned int)*BUFFER.width*BUFFER.height);
	memcpy(BUFFER.zbuf, BUFFER.zbufmin, sizeof(int)*BUFFER.width*BUFFER.height);

	BUFFER.curbuf += 1;
	if(BUFFER.curbuf >= BUFFER.bufcnt)
		BUFFER.curbuf = 0;
	frametime_update();

}
void buf_px(int x, int y, unsigned int color)
{
	if(x<0 || y<0 || x>=BUFFER.width || y>=BUFFER.height)
		return;
	BUFFER.buffers[BUFFER.curbuf][y*BUFFER.width+x] = color;
}
int buf_getz(int x, int y)
{
	if(x<0 || y<0 || x>=BUFFER.width || y>=BUFFER.height)
		return ZBUF_MIN;
	else
		return BUFFER.zbuf[y*BUFFER.width+x];
}
void buf_setz(int x, int y, int z)
{
	if(x<0 || y<0 || x>=BUFFER.width || y>=BUFFER.height)
		return;
	BUFFER.zbuf[y*BUFFER.width+x] = z;
}

void zbuf_to_tga(const char *filename)
{
	char header[18] = {0};
	short width = (short)BUFFER.width;
	short height = (short)BUFFER.height;
	char *colordata = calloc(sizeof(unsigned char), BUFFER.width*BUFFER.height * 4);

	FILE *fp = fopen(filename, "wb");
	
	if(!fp)
	{
		printf("Can't open file %s\n", filename);
		return;
	}

	memcpy(&header[12], &width, sizeof(char)*2);
	memcpy(&header[14], &height, sizeof(char)*2);
	header[2] = 2; 		// image type
	header[16] = 32; 	// BPP

	int cnt = 0;
	for(int i=0;i< BUFFER.width*BUFFER.height;i++)
	{
		colordata[cnt++] = (unsigned char)BUFFER.zbuf[i];
		colordata[cnt++] = (unsigned char)BUFFER.zbuf[i];
		colordata[cnt++] = (unsigned char)BUFFER.zbuf[i];
		colordata[cnt++] = 255;
	}
	fwrite(header, sizeof(char), 18, fp);
	fwrite(colordata, sizeof(unsigned char), BUFFER.width * BUFFER.height * 4, fp);

	fclose(fp);
	free(colordata);
	
	return;
}

void frametime_update()
{
	static int lasttime;
	int curtime;
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	curtime = (int)(ts.tv_sec*1000 + ts.tv_nsec/1000000);
	FRAMETIME = curtime - lasttime;
	lasttime = curtime;
}

void text_draw(int x, int y, const char *text, unsigned int color)
{
	unsigned long long c = 0;

	int ox = x;
	int i = 0;
	while(text[i])
	{
		if(text[i] == '\n')
		{
			x = ox;
			y += TEXTRES;
		}
		else
		{
			c = FONT[(unsigned char)text[i]];
			for(int yi=TEXTRES;yi>0;yi--)
			{
				for(int xi=TEXTRES;xi>0;xi--)
				{
					if(c & 1)
						buf_px(x+xi,y+yi, color);
					c >>= 1;
				}
			}
			x += TEXTRES;
		}
		i++;
	}
}

void input_init()
{
	INPUTS.fd_kb = open(KB_NAME, O_RDONLY);
	INPUTS.fd_mouse = open(MOUSE_NAME, O_RDONLY);
	fcntl(INPUTS.fd_mouse, F_SETFL, O_NONBLOCK);

	if(INPUTS.fd_kb <= 0)
	{
		printf("ERROR: cannot open input: %s\n", KB_NAME);
		return;
	}

	memset(INPUTS.keys, 0, sizeof(INPUTS.keys));

	if(INPUTS.fd_mouse<= 0)
	{
		printf("ERROR: cannot open input: %s\n", MOUSE_NAME);
		return;
	}
	memset(INPUTS.mousedata, 0, sizeof(INPUTS.mousedata));
	INPUTS.mouseactivity = 0;
	INPUTS.mouseshow = 0;
	INPUTS.mousex = 0;
	INPUTS.mousey = 0;

	input_flush();
}
void input_free()
{
	close(INPUTS.fd_kb);
	close(INPUTS.fd_mouse);
}
void input_flush()
{
	memset(INPUTS.mousedata, 0, sizeof(INPUTS.mousedata));
	ioctl(INPUTS.fd_kb, EVIOCGKEY(sizeof(INPUTS.keys)), INPUTS.keys);

	INPUTS.mouseactivity = read(INPUTS.fd_mouse, INPUTS.mousedata, sizeof(INPUTS.mousedata));

	if(INPUTS.mouseshow)
	{
		INPUTS.mousex += input_mouse_relx();
		INPUTS.mousey += input_mouse_rely();
	}
}
int input_key(int key)
{
	return !!(INPUTS.keys[key/8] & (1<<(key%8)));
}
int input_mouse_button(char button)
{
	return INPUTS.mousedata[0] & button;
}
int input_mouse_relx()
{
	return (int)INPUTS.mousedata[1];
}
int input_mouse_rely()
{
	return (int)INPUTS.mousedata[2];
}
int input_mouse_absx()
{
	return INPUTS.mousex;
}
int input_mouse_absy()
{
	return INPUTS.mousey;
}

void camera_update_mat(camera *cam)
{
	cam->fin = mat_mul(cam->proj, cam->vp);
	cam->fin = mat_mul(cam->mv, cam->fin);
}
void camera_angle_from_target(camera *cam)
{
/*
	float dx = cam->target.x - cam->pos.x;
	float dy = cam->target.y - cam->pos.y;
	float dz = cam->target.z - cam->pos.z;

	float distx = sqrtf(dx*dx+dz*dz);
	float disty = sqrtf(dx*dx+dy*dy);

	cam->angle.x = asinf((float)fabs(dx)/distx);
	cam->angle.y = -asinf((float)fabs(dy)/disty);
*/
}
void camera_target_from_angle(camera *cam)
{
	cam->target.x = sinf(cam->angle.x);
	cam->target.y = cosf(cam->angle.x);
	cam->target.z = sinf(cam->angle.y);

	cam->target = vec_add(cam->target, cam->pos);

}

void line(vec2i a, vec2i b, unsigned int color)
{
	int steep = 0;
	int tmp;
	int dx;
	int dy;
	int de2;
	int e2;
	int y;

	if(abs(a.x-b.x) < abs(a.y-b.y))
	{
		tmp = a.y;
		a.y = a.x;
		a.x = tmp;
		tmp = b.y;
		b.y = b.x;
		b.x = tmp;
		steep = 1;
	}
	if(a.x > b.x)
	{
		tmp = a.x;
		a.x = b.x;
		b.x = tmp;
		tmp = a.y;
		a.y = b.y;
		b.y = tmp;
	}
	dx = b.x-a.x;
	dy = b.y-a.y;
	de2 = abs(dy)*2;
	e2 = 0;
	y = a.y;
	for(int x=a.x; x<=b.x;x++)
	{
		if(steep)
			buf_px(y,x,color);
		else
			buf_px(x,y,color);
		e2 += de2;
		if(e2 > dx)
		{
			y += (b.y>a.y?1:-1);
			e2 -= dx*2;
		}
	}
}
void line_dot(vec2i a, vec2i b, unsigned int color)
{
	int steep = 0;
	int tmp;
	int dx;
	int dy;
	int de2;
	int e2;
	int y;

	if(abs(a.x-b.x) < abs(a.y-b.y))
	{
		tmp = a.y;
		a.y = a.x;
		a.x = tmp;
		tmp = b.y;
		b.y = b.x;
		b.x = tmp;
		steep = 1;
	}
	if(a.x > b.x)
	{
		tmp = a.x;
		a.x = b.x;
		b.x = tmp;
		tmp = a.y;
		a.y = b.y;
		b.y = tmp;
	}
	dx = b.x-a.x;
	dy = b.y-a.y;
	de2 = abs(dy)*2;
	e2 = 0;
	y = a.y;
	for(int x=a.x; x<=b.x;x++)
	{
		if(x%2)
		{
			if(steep)
				buf_px(y,x,color);
			else
				buf_px(x,y,color);
		}
		e2 += de2;
		if(e2 > dx)
		{
			y += (b.y>a.y?1:-1);
			e2 -= dx*2;
		}
	}
}
void triangle_color(vec3f a, vec3f b, vec3f c, unsigned int color)
{
	vec3f bary;
	float zfac;
	int t_height;
	int s_height;
	float alpha;
	float beta;
	vec2i out1;
	vec2i out2;

	if(a.y>b.y)
		vec3f_swap(&a, &b);
	if(a.y>c.y)
		vec3f_swap(&a, &c);
	if(b.y>c.y)
		vec3f_swap(&b, &c);

	t_height = c.y-a.y;
	if(t_height <= 0)
		return;

	s_height = b.y-a.y+1;
	if(s_height != 0)
	{
		for(int y=a.y;y<=b.y;y++)
		{
			alpha = (float)(y-a.y)/t_height;
			beta = (float)(y-a.y)/s_height;
			out1.x = a.x+(c.x-a.x)*alpha;
			out1.y = a.y+(c.y-a.y)*alpha;
			out2.x = a.x+(b.x-a.x)*beta;
			out2.y = a.y+(b.y-a.y)*beta;

			if(out1.x>out2.x)
				vec2i_swap(&out1, &out2);

			for(int j=out1.x;j<=out2.x;j++)
			{
				bary = barycentric(a,b,c,(vec3f){j,y,0.0f});
				zfac = 0;
				zfac += bary.x*a.z;
				zfac += bary.y*b.z;
				zfac += bary.z*c.z;
// fix: zfac float => int	
				if(zfac < buf_getz(j,y))
				{
					buf_setz(j,y,zfac);
					buf_px(j,y,color);
				}
			}
		}
	}
	s_height = c.y-b.y+1;
	if(s_height != 0)
	{
		for(int y=b.y;y<=c.y;y++)
		{
			alpha = (float)(y-a.y)/t_height;
			beta = (float)(y-b.y)/s_height;
			out1.x = a.x+(c.x-a.x)*alpha;
			out1.y = a.y+(c.y-a.y)*alpha;
			out2.x = b.x+(c.x-b.x)*beta;
			out2.y = b.y+(c.y-b.y)*beta;
			if(out1.x>out2.x)
				vec2i_swap(&out1, &out2);
				 
			for(int j=out1.x;j<=out2.x;j++)
			{
				bary = barycentric(a,b,c,(vec3f){j,y,0.0f});
				zfac = 0;
				zfac += bary.x*a.z;
				zfac += bary.y*b.z;
				zfac += bary.z*c.z;
				
// fix: zfac float => int	
				if(zfac < buf_getz(j,y))
				{
					buf_setz(j,y,zfac);
					buf_px(j,y,color);
				}
			}
		}
	}
}
void triangle_tex(vec3i a, vec3i b, vec3i c, vec2f uva, vec2f uvb, vec2f uvc, float bright, texture t)
{
	vec2i pos;
	vec2f uvpos;
	vec2i uvi;
	vec2f uvout1;
	vec2f uvout2;
	vec3i out1;
	vec3i out2;
	int t_height;
	int s_height;
	float alpha;
	float beta;
	float phi;
	float zfac;
	unsigned int color;
	int half;

	if((a.y == b.y) && (a.y == c.y))
		return;
	if(a.y>b.y)
	{
		vec3i_swap(&a, &b);
		vec2f_swap(&uva, &uvb);
	}
	if(a.y>c.y)
	{
		vec3i_swap(&a, &c);
		vec2f_swap(&uva, &uvc);
	}
	if(b.y>c.y)
	{
		vec3i_swap(&b, &c);
		vec2f_swap(&uvb, &uvc);
	}

	t_height = c.y-a.y+1;

	for(int i=0;i<t_height;i++)
	{
		half = i > b.y - a.y || b.y == a.y;
		alpha = (float)i/(float)t_height;
		out1.x = a.x+(c.x-a.x)*alpha;
		out1.y = a.y+(c.y-a.y)*alpha;
		out1.z = a.z+(c.z-a.z)*alpha;

		uvout1.x = uva.x + (uvc.x-uva.x)*alpha;
		uvout1.y = uva.y + (uvc.y-uva.y)*alpha;

		if(half)
		{
			s_height = c.y-b.y+1;
			beta = (float) (i-(b.y-a.y))/s_height;
			out2.x = b.x+(c.x-b.x)*beta;
			out2.y = b.y+(c.y-b.y)*beta;
			out2.z = b.z+(c.z-b.z)*beta;

			uvout2.x = uvb.x + (uvc.x-uvb.x)*beta;
			uvout2.y = uvb.y + (uvc.y-uvb.y)*beta;
		}
		else
		{
			s_height = b.y-a.y+1;
			beta = (float)i/(float)s_height;
			out2.x = a.x+(b.x-a.x)*beta;
			out2.y = a.y+(b.y-a.y)*beta;
			out2.z = a.z+(b.z-a.z)*beta;

			uvout2.x = uva.x + (uvb.x-uva.x)*beta;
			uvout2.y = uva.y + (uvb.y-uva.y)*beta;
		}

		if(out1.x>out2.x)
		{
			vec3i_swap(&out1, &out2);
			vec2f_swap(&uvout1, &uvout2);
		}

		for(int j=out1.x;j<=out2.x;j++)
		{
			if(j<0 || j>=BUFFER.width)
				continue;
			if(out1.x == out2.x)
				phi = 1.0f;
			else
				phi = (float)(j-out1.x)/(out2.x-out1.x);

			pos = (vec2i){j, a.y+i};
			zfac = out1.z + (out2.z-out1.z)*phi;
// fix: zfac float => int	
			if(buf_getz(pos.x,pos.y) < zfac)
			{
				uvpos.x = uvout1.x + (uvout2.x-uvout1.x)*phi;
				uvpos.y = uvout1.y + (uvout2.y-uvout1.y)*phi;
// FIXME sanitize uv and remove these
if(uvpos.x < 0.0f)
	uvpos.x = 0.0f;
if(uvpos.y < 0.0f)
	uvpos.y = 0.0f;
if(uvpos.x > 1.0f)
	uvpos.x = 1.0f;
if(uvpos.y > 1.0f)
	uvpos.y = 1.0f;

				uvi.x = (int)(t.width*uvpos.x);
				uvi.y = (int)(t.height*uvpos.y);


				color = t.data[uvi.x+uvi.y*t.width];
				buf_setz(pos.x,pos.y,zfac);
				buf_px(pos.x,pos.y,color);
			}
		}
	}
}
void rect(vec2i a, vec2i b, unsigned int color)
{
	for(int y=a.y;y<a.y+b.y;y++)
	{
		for(int x=a.x;x<a.x+b.x;x++)
		{
			buf_px(x,y,color);
		}
	}
}
int triangle_in_viewport(vec3i a, vec3i b, vec3i c)
{
// FIXME NEED TESTS still droppin triangles
	if((a.z < 0) && (b.z < 0) && (c.z < 0))
		return 0;
	if((a.y < 0) && (b.y < 0) && (c.y < 0))
		return 0;
	if((a.y > BUFFER.height) && (b.y > BUFFER.height) && (c.y > BUFFER.height))
		return 0;

	if((a.x < 0) && (b.x < 0) && (c.x < 0))
		return 0;
	if((a.x > BUFFER.width) && (b.x > BUFFER.width) && (c.x > BUFFER.width))
		return 0;

	return 1;
}

model loadiqe(const char *filename)
{
	char line[512];
	model out = {0};
	FILE *fp = fopen(filename, "r");

	if(!fp)
	{
		printf("Can't open file %s\n", filename);
		return out;
	}
// first pass
	while(fgets(line, 512, fp) != NULL)
	{
		if(line[0] == 'v' && line[1] == 'p')
			out.vcnt++;
		if(line[0] == 'f' && line[1] == 'm')
			out.fcnt++;
	}
	out.vp = malloc(sizeof(vec3f)*out.vcnt);
	out.vt = malloc(sizeof(vec2f)*out.vcnt);
	out.vn = malloc(sizeof(vec3f)*out.vcnt);
	out.fm = malloc(sizeof(int)*out.fcnt*3);
	out.fn = malloc(sizeof(vec3f)*out.fcnt);
	rewind(fp);
// second pass
	int vpcnt = 0;
	int vtcnt = 0;
	int vncnt = 0;
	int fmcnt = 0;
	vec3f invec;
	int inint[3];
	while(fgets(line, 512, fp) != NULL)
	{
		if(sscanf(line," vp %f %f %f", &invec.x, &invec.y, &invec.z) == 3)
		{
			out.vp[vpcnt] = invec;
			vpcnt++;
		}
		else if(sscanf(line," vt %f %f", &invec.x, &invec.y) == 2)
		{
			out.vt[vtcnt].x = invec.x;
			out.vt[vtcnt].y = 1.0f-invec.y;
			vtcnt++;
		}
		else if(sscanf(line," vn %f %f %f", &invec.x, &invec.y, &invec.z) == 3)
		{
			out.vn[vncnt] = invec;
			vncnt++;
		}
		else if(sscanf(line," fm %i %i %i", &inint[0], &inint[1], &inint[2]) == 3)
		{
			out.fm[fmcnt] = inint[0];
			fmcnt++;
			out.fm[fmcnt] = inint[1];
			fmcnt++;
			out.fm[fmcnt] = inint[2];
			fmcnt++;
		}
	}
// calculate normals
	vec3f a;
	vec3f b;
	vec3f c;
	for(int f=0;f<out.fcnt;f++)
	{
		a = out.vp[out.fm[f*3+0]];
		b = out.vp[out.fm[f*3+1]];
		c = out.vp[out.fm[f*3+2]];

		out.fn[f] = vec_cross(vec_sub(c,a),vec_sub(b,a));
		out.fn[f] = vec_norm(out.fn[f]);
	}
	fclose(fp);
	return out;
}
void unloadmodel(model m)
{
	free(m.vp);
	free(m.vt);
	free(m.vn);
	free(m.fm);
	free(m.fn);
}

void drawmodel_wire(model m, unsigned int color)
{
	vec3f in[CLIP_POINT_IN];
	vec3f out[CLIP_POINT_OUT];
	vec2f uvin[CLIP_POINT_IN];
	vec2f uvout[CLIP_POINT_OUT];
	int outcnt = 0;

	vec2i ai;
	vec2i bi;
	vec2i ci;
	vec3f a;
	vec3f b;
	vec3f c;
	vec3f n;

	const vec3f l = (vec3f){0.0f,0.0f,-1.0f};

	float face = 0.0f;
	unsigned int colorout;
	for(int f=0;f<m.fcnt;f++)
	{
		in[0] = vec_trans(m.vp[m.fm[f*3+0]], CAMERA->fin);
		in[1] = vec_trans(m.vp[m.fm[f*3+1]], CAMERA->fin);
		in[2] = vec_trans(m.vp[m.fm[f*3+2]], CAMERA->fin);
		n = vec_norm(vec_cross(vec_sub(c, a), vec_sub(b, a)));
		face = vec_dot(n, l);

// pre-clip
			ai = (vec2i){(int)in[0].x, (int)in[0].y};
			bi = (vec2i){(int)in[1].x, (int)in[1].y};
			ci = (vec2i){(int)in[2].x, (int)in[2].y};
			colorout = color^0x555555;
			line(ai,bi,colorout);
			line(bi,ci,colorout);
			line(ci,ai,colorout);

//memcpy(out, in, sizeof(vec3f) * 3);
//outcnt = 1;
triangle_clip_viewport(in, uvin, out, uvout, &outcnt);
		for(int i=0; i < outcnt; i++)
		{
			ai = (vec2i){(int)out[i*3+0].x, (int)out[i*3+0].y};
			bi = (vec2i){(int)out[i*3+1].x, (int)out[i*3+1].y};
			ci = (vec2i){(int)out[i*3+2].x, (int)out[i*3+2].y};

			if(face > 0.0f)
				colorout = color;
			else
				colorout = color^0xffffff;
			line(ai,bi,colorout);
			line(bi,ci,colorout);
			line(ci,ai,colorout);
		}
	}
}
void drawmodel_tex(model m, texture t)
{
	vec3f in[CLIP_POINT_IN];
	vec3f out[CLIP_POINT_OUT];
	vec2f uvin[CLIP_POINT_IN];
	vec2f uvout[CLIP_POINT_OUT];
	
	vec3f n;

	vec3i ai;
	vec3i bi;
	vec3i ci;

	vec2f uva;
	vec2f uvb;
	vec2f uvc;

	const vec3f l = (vec3f){0.0f,0.0f,-1.0f};

	int outcnt = 0;
	float face = 0.0f;
	for(int f=0;f<m.fcnt;f++)
	{
		in[0] = vec_trans(m.vp[m.fm[f*3+0]], CAMERA->fin);
		in[1] = vec_trans(m.vp[m.fm[f*3+1]], CAMERA->fin);
		in[2] = vec_trans(m.vp[m.fm[f*3+2]], CAMERA->fin);
		n = m.fn[f];
		n = vec_norm(vec_cross(vec_sub(in[2], in[0]),vec_sub(in[1], in[0])));
		face = vec_dot(n,l);

		if(face < 0.0f)
		{

			uvin[0] = m.vt[m.fm[f*3+0]];
			uvin[1] = m.vt[m.fm[f*3+1]];
			uvin[2] = m.vt[m.fm[f*3+2]];
if(in[0].z < 0.0f || in[1].z < 0.0f || in[2].z < 0.0f)
	continue;
memcpy(out, in, sizeof(vec3f) * 3);
memcpy(uvout, uvin, sizeof(vec2f) * 3);
outcnt = 1;
			/*
			triangle_clip_viewport(in, uvin, out, uvout, &outcnt);
			printf("%f %f %f\n", in[0].z, in[1].z, in[2].z);
			for(int i=0;i< outcnt;i++) {
				printf("%f %f %f\n", out[3*i+0].z, out[3*i+1].z, out[3*i+2].z);
			}
			printf("outcnt: %i\n\n", outcnt);
			*/
			for(int i=0; i < outcnt; i++)
			{
				ai = (vec3i){out[0].x, out[0].y, out[0].z};
				bi = (vec3i){out[1].x, out[1].y, out[1].z};
				ci = (vec3i){out[2].x, out[2].y, out[2].z};

				uva = uvout[0];
				uvb = uvout[1];
				uvc = uvout[2];

				triangle_tex(ai,bi,ci,uva,uvb,uvc,face,t);
			}
		}
	}
}

texture loadtga(const char *filename)
{
	texture out = {0};
	char header[18];

	FILE *fp = fopen(filename,"rb");

	if(!fp)
	{
		printf("Can't open file %s\n", filename);
		return out;
	}

	char bbp;
	char *colordata = 0;

	if(fread(&header, sizeof(header), 1, fp) == 0)
	{
		printf("Can't read TGA header from file %s\n", filename);
		return out;
	}
	memcpy(&out.width, &header[12], sizeof(char)*2);
	memcpy(&out.height, &header[14], sizeof(char)*2);
	bbp = header[16];
// TODO: header sanity checking
	out.data = calloc(sizeof(unsigned int),out.width*out.height);
	colordata = calloc(sizeof(unsigned char),out.width*out.height*bbp);
	if(fread(colordata, sizeof(unsigned char), out.width*out.height*bbp, fp) == 0)
	{
		printf("Can't read TGA color data from file %s\n", filename);
		return out;
	}
	int charcnt = 0;
	for(int i=0;i<out.width*out.height;i++)
	{
		out.data[i] = colordata[charcnt++] << BSHIFT;
		out.data[i] += colordata[charcnt++] << GSHIFT;
		out.data[i] += colordata[charcnt++] << RSHIFT;
		if(bbp == 32)
			charcnt++;
	}
	free(colordata);
	fclose(fp);
	return out;
}
void unloadtex(texture t)
{
	free(t.data);
}
void drawtex(texture t)
{
	for(int y=0; y<t.height;y++)
	{
		for(int x=0; x<t.width;x++)
		{
			buf_px(x,y,t.data[x+y*t.width]);
		}
	}
}

void triangle_clip_viewport(vec3f *posin, vec2f *uvin, vec3f *posout, vec2f *uvout, int *cntout)
{
	if(posin[0].z <= 0.0f && posin[1].z <= 0.0f && posin[2].z <= 0.0f)
	{
		*cntout = 0;
		return;
	}
	/*
	else if(posin[0].z <= 0.0f)
	{
		if(posin[1].z <= 0.0f)
		{
			*cntout = 1;
			triangle_clip_double(posin[2], posin[0], posin[1], uvin[2], uvin[0], uvin[1], posout, uvout);
			return;
		}
		else if(posin[2].z <= 0.0f)
		{
			*cntout = 1;
			triangle_clip_double(posin[1], posin[0], posin[2], uvin[1], uvin[0], uvin[2], posout, uvout);
			return;
		}
		else
		{
			*cntout = 2;
			triangle_clip_single(posin[1], posin[2], posin[0], uvin[1], uvin[2], uvin[0], posout, uvout);
			return;
		}
	}
	else if(posin[1].z <= 0.0f)
	{
		if(posin[2].z <= 0.0f)
		{
			*cntout = 1;
			triangle_clip_double(posin[0], posin[1], posin[2], uvin[0], uvin[1], uvin[2], posout, uvout);
			return;
		}
		else
		{
			*cntout = 2;
			triangle_clip_single(posin[0], posin[2], posin[1], uvin[0], uvin[2], uvin[1], posout, uvout);
			return;
		}
	}
	*/
	else if(posin[2].z <= 0.0f)
	{
		*cntout = 2;
		triangle_clip_single(posin[0], posin[1], posin[2], uvin[0], uvin[1], uvin[2], posout, uvout);
		return;
	}
	else
	{
		memcpy(posout, posin, sizeof(vec3f) * CLIP_POINT_IN);
		memcpy(uvout, uvin, sizeof(vec2f) * CLIP_POINT_IN);
		*cntout = 1;
		return;
	}
}
void triangle_clip_single(vec3f in1, vec3f in2, vec3f out, vec2f in1uv, vec2f in2uv, vec2f outuv, vec3f *posout, vec2f *uvout)
{
/*
	float lerp1 = (-out.z) / (in1.z - out.z);
	float lerp2 = (-out.z) / (in2.z - out.z);

*/
	float lerp1 = inv_lerp(out.z, in1.z, CLIP_NEAR);
	float lerp2 = inv_lerp(out.z, in2.z, CLIP_NEAR);

	posout[0] = vec3f_lerp(out, in1, lerp1);
	posout[1] = in1;
	posout[2] = in2;
	uvout[0] = vec2f_lerp(outuv, in1uv, lerp1);
	uvout[1] = in1uv;
	uvout[2] = in2uv;

	posout[3] = vec3f_lerp(out, in2, lerp2);
	posout[4] = posout[0];
	posout[5] = in2;
	uvout[3] = vec2f_lerp(outuv, in2uv, lerp2);
	uvout[4] = uvout[0];
	uvout[5] = in2uv;

printf("LERP:\n");
	printf("1: %f\n", lerp1);
	printf("2: %f\n", lerp2);
printf("IN:\n");
	printf("0: %f %f %f\n", in1.x,in1.y,in1.z);
	printf("1: %f %f %f\n", in2.x,in2.y,in2.z);
	printf("2: %f %f %f\n", out.x,out.y,out.z);
printf("OUT:\n");
	printf("0: %f %f %f\n", posout[0].x,posout[0].y,posout[0].z);
	printf("1: %f %f %f\n", posout[1].x,posout[1].y,posout[1].z);
	printf("2: %f %f %f\n", posout[2].x,posout[2].y,posout[2].z);
	printf("3: %f %f %f\n", posout[3].x,posout[3].y,posout[3].z);
	printf("4: %f %f %f\n", posout[4].x,posout[4].y,posout[4].z);
	printf("5: %f %f %f\n", posout[5].x,posout[5].y,posout[5].z);

}
void triangle_clip_double(vec3f in, vec3f out1, vec3f out2, vec2f inuv, vec2f out1uv, vec2f out2uv, vec3f *posout, vec2f *uvout)
{
	float lerp1 = (-out1.z) / (in.z - out1.z);
	float lerp2 = (-out2.z) / (in.z - out2.z);

	posout[0] = vec3f_lerp(out1, in, lerp1);
	posout[1] = vec3f_lerp(out2, in, lerp2);
	posout[2] = in;
	uvout[0] = vec2f_lerp(out1uv, inuv, lerp1);
	uvout[1] = vec2f_lerp(out2uv, inuv, lerp2);
	uvout[2] = inuv;
}

vec3f vec_cross(vec3f a, vec3f b)
{
	vec3f out = {a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x};
	return out;
}
float vec_dot(vec3f a, vec3f b)
{
	return a.x*b.x+a.y*b.y+a.z*b.z;
}
float vec_len(vec3f a)
{
	return sqrtf(a.x*a.x + a.y*a.y + a.z*a.z);
}
int vec_dot_i(vec3i a, vec3i b)
{
	return a.x*b.x+a.y*b.y+a.z*b.z;
}
float vec_dist(vec3f a, vec3f b)
{
	float dx = b.x-a.x;
	float dy = b.y-a.y;
	float dz = b.z-a.z;

	return sqrtf(dx*dx + dy*dy + dz*dz);
}
vec3f vec_add(vec3f a, vec3f b)
{
	vec3f out;
	out.x = a.x+b.x;
	out.y = a.y+b.y;
	out.z = a.z+b.z;
	return out;
}
vec3f vec_sub(vec3f a, vec3f b)
{
	vec3f out;
	out.x = a.x-b.x;
	out.y = a.y-b.y;
	out.z = a.z-b.z;
	return out;
}
vec3i vec_sub_i(vec3i a, vec3i b)
{
	vec3i out;
	out.x = a.x-b.x;
	out.y = a.y-b.y;
	out.z = a.z-b.z;
	return out;
}
vec3f vec_mul_f(vec3f a, float f)
{
	return (vec3f){a.x*f, a.y*f, a.z*f};
}
vec3f vec_norm(vec3f a)
{
	float l = sqrtf(a.x*a.x+a.y*a.y+a.z*a.z);
	float il;
	if(l == 0.0f)
		l = 1.0f;
	il = 1.0f/l;
	a.x *= il;
	a.y *= il;
	a.z *= il;
	return a;
}
vec3f barycentric(vec3f a, vec3f b, vec3f c, vec3f p)
{
	vec3f out = {0};
	vec3f v0 = vec_sub(b,a);
	vec3f v1 = vec_sub(c,a);
	vec3f v2 = vec_sub(p,a);
	float d00 = vec_dot(v0,v0);
	float d01 = vec_dot(v0,v1);
	float d11 = vec_dot(v1,v1);
	float d20 = vec_dot(v2,v0);
	float d21 = vec_dot(v2,v1);
	float denominv = 1.0f/(d00*d11 - d01*d01);
	out.x = (d11*d20 - d01*d21)*denominv;
	out.y = (d00*d21 - d01*d20)*denominv;
	out.z = 1.0f-out.x-out.y;
	return out;
}
vec3f barycentric_i(vec3i a, vec3i b, vec3i c, vec3i p)
{
	vec3f out = {0};
	vec3i v0 = vec_sub_i(b,a);
	vec3i v1 = vec_sub_i(c,a);
	vec3i v2 = vec_sub_i(p,a);
	int d00 = vec_dot_i(v0,v0);
	int d01 = vec_dot_i(v0,v1);
	int d11 = vec_dot_i(v1,v1);
	int d20 = vec_dot_i(v2,v0);
	int d21 = vec_dot_i(v2,v1);
	int denom = d00*d11 - d01*d01;
	out.y = (float)(d11*d20 - d01*d21)/(float)denom;
	out.z = (float)(d00*d21 - d01*d20)/(float)denom;
	out.x = 1.0f-(out.z+out.y);
	return out;
}
vec2f bary2carth(vec2f a, vec2f b, vec2f c, vec3f p)
{
	vec2f out = {0};
	out.x = a.x*p.x+b.x*p.y+c.x*p.z;
	out.y = a.y*p.x+b.y*p.y+c.y*p.z;
	if(out.x < 0.0f)
		out.x = 0.0f;
	if(out.y < 0.0f)
		out.y = 0.0f;
	if(out.x > 1.0f)
		out.x = 1.0f;
	if(out.y > 1.0f)
		out.y = 1.0f;
	return out;
}
vec3f vec_trans(vec3f a, mat4f m)
{
	vec3f out = {0};
	float w;

	out.x = m.m0*a.x + m.m4*a.y + m.m8*a.z + m.m12;
	out.y = m.m1*a.x + m.m5*a.y + m.m9*a.z + m.m13;
	out.z = m.m2*a.x + m.m6*a.y + m.m10*a.z + m.m14;
	w = m.m3*a.x + m.m7*a.y + m.m11*a.z + m.m15;

	out.x = out.x/w;
	out.y = out.y/w;
	out.z = out.z/w;

	return out;
}

void vec2i_swap(vec2i *a, vec2i *b)
{
	vec2i tmp = *a;
	*a = *b;
	*b = tmp;
}
void vec2f_swap(vec2f *a, vec2f *b)
{
	vec2f tmp = *a;
	*a = *b;
	*b = tmp;
}
void vec3i_swap(vec3i *a, vec3i *b)
{
	vec3i tmp = *a;
	*a = *b;
	*b = tmp;
}
void vec3f_swap(vec3f *a, vec3f *b)
{
	vec3f tmp = *a;
	*a = *b;
	*b = tmp;
}

vec3f vec3f_lerp(vec3f a, vec3f b, float amt)
{
	vec3f out = {0};
	out.x = lerp(a.x, b.x, amt);
	out.y = lerp(a.y, b.y, amt);
	out.z = lerp(a.z, b.z, amt);
	return out;
}
vec3i vec3i_lerp(vec3i a, vec3i b, float amt)
{
	vec3i out = {0};
	out.x = lerp_i(a.x, b.x, amt);
	out.y = lerp_i(a.y, b.y, amt);
	out.z = lerp_i(a.z, b.z, amt);
	return out;
}
vec2f vec2f_lerp(vec2f a, vec2f b, float amt)
{
	vec2f out = {0};
	out.x = lerp(a.x, b.x, amt);
	out.y = lerp(a.y, b.y, amt);
	return out;
}
float lerp(float a, float b, float amt)
{
	return (float)a+amt*(b-a);
}
float inv_lerp(float a, float b, float c)
{
	return (float)(c-a)/(b-a);
}
float lerp_i(int a, int b, float amt)
{
	return (int)(a+amt*(b-a));
}

mat4f mat_identity()
{
	mat4f out = {1.0f, 0.0f, 0.0f, 0.0f,
				 0.0f, 1.0f, 0.0f, 0.0f,
				 0.0f, 0.0f, 1.0f, 0.0f,
				 0.0f, 0.0f, 0.0f, 1.0f};
	return out;
}
mat4f viewport(int x, int y, int w, int h)
{
	mat4f out = mat_identity();
	out.m12 = x+w/2.0f;
	out.m13 = y+h/2.0f;
	out.m14 = DEPTH;

	out.m0 = w/2.0f;
	out.m5 = h/2.0f;
	out.m10 = DEPTH;
	return out;
}
mat4f mat_mul(mat4f a, mat4f b)
{
	mat4f out = {0};
	out.m0 = a.m0*b.m0 + a.m1*b.m4 + a.m2*b.m8 + a.m3*b.m12;
	out.m1 = a.m0*b.m1 + a.m1*b.m5 + a.m2*b.m9 + a.m3*b.m13;
	out.m2 = a.m0*b.m2 + a.m1*b.m6 + a.m2*b.m10 + a.m3*b.m14;
	out.m3 = a.m0*b.m3 + a.m1*b.m7 + a.m2*b.m11 + a.m3*b.m15;

	out.m4 = a.m4*b.m0 + a.m5*b.m4 + a.m6*b.m8 + a.m7*b.m12;
	out.m5 = a.m4*b.m1 + a.m5*b.m5 + a.m6*b.m9 + a.m7*b.m13;
	out.m6 = a.m4*b.m2 + a.m5*b.m6 + a.m6*b.m10 + a.m7*b.m14;
	out.m7 = a.m4*b.m3 + a.m5*b.m7 + a.m6*b.m11 + a.m7*b.m15;

	out.m8 = a.m8*b.m0 + a.m9*b.m4 + a.m10*b.m8 + a.m11*b.m12;
	out.m9 = a.m8*b.m1 + a.m9*b.m5 + a.m10*b.m9 + a.m11*b.m13;
	out.m10 = a.m8*b.m2 + a.m9*b.m6 + a.m10*b.m10 + a.m11*b.m14;
	out.m11 = a.m8*b.m3 + a.m9*b.m7 + a.m10*b.m11 + a.m11*b.m15;

	out.m12 = a.m12*b.m0 + a.m13*b.m4 + a.m14*b.m8 + a.m15*b.m12;
	out.m13 = a.m12*b.m1 + a.m13*b.m5 + a.m14*b.m9 + a.m15*b.m13;
	out.m14 = a.m12*b.m2 + a.m13*b.m6 + a.m14*b.m10 + a.m15*b.m14;
	out.m15 = a.m12*b.m3 + a.m13*b.m7 + a.m14*b.m11 + a.m15*b.m15;
	return out;
}
mat4f mat_lookat(vec3f pos, vec3f tar, vec3f up)
{
// TODO: optimize
	mat4f minv = mat_identity();
	mat4f tr = mat_identity();

	vec3f z = vec_norm(vec_sub(pos, tar));
	vec3f x = vec_norm(vec_cross(up, z));
	vec3f y = vec_norm(vec_cross(z,x));

	minv.m0 = x.x;
	minv.m1 = y.x;
	minv.m2 = z.x;
	minv.m3 = 0.0f;

	minv.m4 = x.y;
	minv.m5 = y.y;
	minv.m6 = z.y;
	minv.m7 = 0.0f;

	minv.m8 = x.z;
	minv.m9 = y.z;
	minv.m10 = z.z;
	minv.m11 = 0.0f;

	tr.m12 = -tar.x;
	tr.m13 = -tar.y;
	tr.m14 = -tar.z;

	mat4f out = mat_mul(tr,minv);

	return out;
}

unsigned int color_rgb(unsigned int r, unsigned int g, unsigned int b)
{
	unsigned int color = r << RSHIFT;
	color += g << GSHIFT;
	color += b << BSHIFT;
	return color;
}
unsigned int brighten(unsigned int c, float b)
{
	unsigned int out;
	unsigned int a;
	a = c & 0xFF;
	out = (int)(a*b) << BSHIFT;
	a = (c & 0xFF00) >> GSHIFT;
	out += (int)(a*b) << GSHIFT;
	a = (c & 0xFF0000) >> RSHIFT;
	out += (int)(a*b) << RSHIFT;
	return out;
}

void print_mat(mat4f m)
{
	printf("%f %f %f %f\n", m.m0, m.m4, m.m8, m.m12);
	printf("%f %f %f %f\n", m.m1, m.m5, m.m9, m.m13);
	printf("%f %f %f %f\n", m.m2, m.m6, m.m10, m.m14);
	printf("%f %f %f %f\n", m.m3, m.m7, m.m11, m.m15);
}
