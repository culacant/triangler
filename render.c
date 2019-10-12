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

	BUFFER.zbuf = malloc(sizeof(float) * BUFFER.width*BUFFER.height);
	BUFFER.zbufmax = malloc(sizeof(float) * BUFFER.width*BUFFER.height);

	for(int i=0;i<BUFFER.width*BUFFER.height;i++)
		BUFFER.zbufmax[i] = ZBUF_MAX;

	memcpy(BUFFER.zbuf, BUFFER.zbufmax, sizeof(float)*BUFFER.width*BUFFER.height);
	frametime_update();
}
void buf_free()
{
	for(int i=0;i<BUFFER.bufcnt;i++)
		free(BUFFER.buffers[i]);
	free(BUFFER.buffers);
	free(BUFFER.zbuf);
	free(BUFFER.zbufmax);
	munmap(BUFFER.buf,4*BUFFER.width*BUFFER.height);
	close(BUFFER.fd);
}
void buf_flush()
{
	memcpy(BUFFER.buf, BUFFER.buffers[BUFFER.curbuf], sizeof(unsigned int)*BUFFER.width*BUFFER.height);
	memset(BUFFER.buffers[BUFFER.curbuf], 0, sizeof(unsigned int)*BUFFER.width*BUFFER.height);
	memcpy(BUFFER.zbuf, BUFFER.zbufmax, sizeof(float)*BUFFER.width*BUFFER.height);

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
float buf_getz(int x, int y)
{
	if(x<0 || y<0 || x>=BUFFER.width || y>=BUFFER.height)
		return ZBUF_MAX;
	else
		return BUFFER.zbuf[y*BUFFER.width+x];
}
void buf_setz(int x, int y, float z)
{
	if(x<0 || y<0 || x>=BUFFER.width || y>=BUFFER.height)
		return;
	BUFFER.zbuf[y*BUFFER.width+x] = z;
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
/*
	int yi = 0;
	int yl = 0;
	int xi = 0;
*/
	unsigned long long c = 0;

	int i = 0;
	while(text[i])
	{
		c = FONT[text[i]];

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
	float dx = cam->target.x - cam->pos.x;
	float dy = cam->target.y - cam->pos.y;
	float dz = cam->target.z - cam->pos.z;

	cam->angle.x = atan2f(dx, dz);
	cam->angle.y = atan2f(dx, dy);
}
void camera_target_from_angle(camera *cam)
{
	cam->target.x = cam->pos.x - sinf(cam->angle.x)*FOCUS_DIST;
	cam->target.y = cam->pos.y + sinf(cam->angle.y)*FOCUS_DIST;
	cam->target.z = cam->pos.z - cosf(cam->angle.y)*FOCUS_DIST;
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
void triangle_color(vec3f a, vec3f b, vec3f c, unsigned int color)
{
	vec3f tmp3;
	vec3f bary;
	float zfac;
	vec2i tmp2;
	int t_height;
	int s_height;
	float alpha;
	float beta;
	vec2i out1;
	vec2i out2;
	if(a.y>b.y)
	{
		tmp3 = a;
		a = b;
		b = tmp3;
	}
	if(a.y>c.y)
	{
		tmp3 = a;
		a = c;
		c = tmp3;
	}
	if(b.y>c.y)
	{
		tmp3 = c;
		c = b;
		b = tmp3;
	}
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
			{
				tmp2 = out1;
				out1 = out2;
				out2 = tmp2;
			}
			for(int j=out1.x;j<=out2.x;j++)
			{
				bary = barycentric(a,b,c,(vec3f){j,y,0.0f});
				zfac = 0;
				zfac += bary.x*a.z;
				zfac += bary.y*b.z;
				zfac += bary.z*c.z;
				
				if(zfac > buf_getz(j,y))
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
			{
				tmp2 = out1;
				out1 = out2;
				out2 = tmp2;
			}
			for(int j=out1.x;j<=out2.x;j++)
			{
				bary = barycentric(a,b,c,(vec3f){j,y,0.0f});
				zfac = 0;
				zfac += bary.x*a.z;
				zfac += bary.y*b.z;
				zfac += bary.z*c.z;
				
				if(zfac > buf_getz(j,y))
				{
					buf_setz(j,y,zfac);
					buf_px(j,y,color);
				}
			}
		}
	}
}
void triangle_tex_i(vec3i a, vec3i b, vec3i c, vec2f uva, vec2f uvb, vec2f uvc, float bright, texture t)
{
	vec3i itmp3;
	vec3f bary;
	vec2f tmp2;
	vec2f uv;
	vec2i uvi;
	vec2i out1;
	vec2i out2;
	vec2i itmp2;
	float zfac;
	int t_height;
	int s_height;
	float alpha;
	float beta;
	unsigned int color;
	int half;

	if(a.y>b.y)
	{
		itmp3 = a;
		a = b;
		b = itmp3;
		tmp2 = uva;
		uva = uvb;
		uvb = tmp2;
	}
	if(a.y>c.y)
	{
		itmp3 = a;
		a = c;
		c = itmp3;
		tmp2 = uva;
		uva = uvc;
		uvc = tmp2;
	}
	if(b.y>c.y)
	{
		itmp3 = c;
		c = b;
		b = itmp3;
		tmp2 = uvc;
		uvc = uvb;
		uvb = tmp2;
	}

	t_height = c.y-a.y+1;
	int i = 0;
	if(a.y < 0)
		i -= a.y;
	int maxi = t_height;
	if(a.y+t_height > BUFFER.height)
		maxi = BUFFER.height - a.y;

	for(;i<maxi;i++)
	{
		half = i > b.y - a.y || b.y == a.y;
		alpha = (float)i/(float)t_height;
		out1.x = a.x+(c.x-a.x)*alpha;
		out1.y = a.y+(c.y-a.y)*alpha;
		if(half)
		{
			s_height = c.y-b.y+1;
			beta = (float) (i-(b.y-a.y))/s_height;
			out2.x = b.x+(c.x-b.x)*beta;
		}
		else
		{
			s_height = b.y-a.y+1;
			beta = (float)i/(float)s_height;
			out2.x = a.x+(b.x-a.x)*beta;
		}
		if(out1.x>out2.x)
		{
			itmp2 = out1;
			out1 = out2;
			out2 = itmp2;
		}
		int j = out1.x;
		int j2 = out2.x;
		if(j < 0)
			j = 0;
		if(j >= BUFFER.width)
			continue;
		if(j2 < 0)
			j2 = 0;
		if(j2 >= BUFFER.width)
			j2 = BUFFER.width;
		for(;j<=j2;j++)
		{
			
			if(j<0 || a.y+i<0 || j>=BUFFER.width || a.y+i>=BUFFER.height)
				continue;

			bary = barycentric_i(a,b,c,(vec3i){j,a.y+i,a.z});
			zfac = bary.x*a.z;
			zfac += bary.y*b.z;
			zfac += bary.z*c.z;
			if(zfac < buf_getz(j,a.y+i))
			{
				uv = bary2carth(uva, uvb, uvc, bary);
				uvi.x = (int)(t.width*uv.x)%t.width;
				uvi.y = (int)(t.height*uv.y)%t.height;
				color = t.data[uvi.x+uvi.y*t.width];
				buf_setz(j,a.y+i,zfac);
				buf_px(j,a.y+i,color);
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
// FIXME
	if( (a.y < 0 && b.y < 0 && c.y < 0) ||
		(a.y > BUFFER.height && b.y > BUFFER.height && c.y > BUFFER.height))
		return 0;
	if(	(a.x < 0 && b.x < 0 && c.x < 0) ||
		(a.x > BUFFER.width && b.x > BUFFER.width && c.x > BUFFER.width))
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
		if(sscanf(line," vp %f %f %f", &invec.x, &invec.z, &invec.y) == 3)
		{
			out.vp[vpcnt] = invec;
			vpcnt++;
		}
		else if(sscanf(line," vt %f %f", &invec.x, &invec.y) == 2)
		{
			out.vt[vtcnt].x = 1.0f-invec.x;
			out.vt[vtcnt].y = 1.0f-invec.y;
			vtcnt++;
		}
		else if(sscanf(line," vn %f %f %f", &invec.x, &invec.z, &invec.y) == 3)
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
	vec2i a;
	vec2i b;
	vec2i c;
	vec3f v0;
	vec3f v1;
	vec3f v2;
	for(int f=0;f<m.fcnt;f++)
	{
		v0 = m.vp[m.fm[f*3+0]];
		v1 = m.vp[m.fm[f*3+1]];
		v2 = m.vp[m.fm[f*3+2]];

		v0 = vec_trans(v0,CAMERA->fin);
		v1 = vec_trans(v1,CAMERA->fin);
		v2 = vec_trans(v2,CAMERA->fin);
		a = (vec2i){(int)v0.x, (int)v0.y};
		b = (vec2i){(int)v1.x, (int)v1.y};
		c = (vec2i){(int)v2.x, (int)v2.y};

		line(a,b,color);
		line(b,c,color);
		line(c,a,color);
	}
}
void drawmodel_tex(model m, texture t)
{
	vec3f a;
	vec3f b;
	vec3f c;
	vec3f n;
	vec2f uva;
	vec2f uvb;
	vec2f uvc;

	vec3i ai;
	vec3i bi;
	vec3i ci;

	vec3f l = (vec3f){0.0f,0.0f,1.0f};

	float face = 0.0f;
	for(int f=0;f<m.fcnt;f++)
	{
		a = m.vp[m.fm[f*3+0]];
		b = m.vp[m.fm[f*3+1]];
		c = m.vp[m.fm[f*3+2]];
		n = m.fn[f];

		a = vec_trans(a,CAMERA->fin);
		b = vec_trans(b,CAMERA->fin);
		c = vec_trans(c,CAMERA->fin);

		ai.x = (int)a.x;
		ai.y = (int)a.y;
		ai.z = (int)a.z;
		bi.x = (int)b.x;
		bi.y = (int)b.y;
		bi.z = (int)b.z;
		ci.x = (int)c.x;
		ci.y = (int)c.y;
		ci.z = (int)c.z;

		if(triangle_in_viewport(ai, bi, ci))
		{
			n = vec_norm(vec_cross(vec_sub(c,a),vec_sub(b,a)));
			face = vec_dot(n,l);

			if(face > 0.0f)
			{
				uva = m.vt[m.fm[f*3+0]];
				uvb = m.vt[m.fm[f*3+1]];
				uvc = m.vt[m.fm[f*3+2]];

				triangle_tex_i(ai,bi,ci,uva,uvb,uvc,face,t);
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
	if(out.x < 0)
		out.x = 0;
	if(out.y < 0)
		out.y = 0;
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
	out.m14 = DEPTH/2.0f;

	out.m0 = w/2.0f;
	out.m5 = h/2.0f;
	out.m10 = DEPTH/2.0f;
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
mat4f mat_lookat(vec3f pos, vec3f center, vec3f up)
{
	mat4f out = {0};
	vec3f z = vec_norm(vec_sub(pos, center));
	vec3f x = vec_norm(vec_cross(up, z));
	vec3f y = vec_norm(vec_cross(z, x));

	out.m0 = x.x;
	out.m4 = x.y;
	out.m8 = x.z;
	out.m12 = -1.0f*center.x;

	out.m1 = y.x;
	out.m5 = y.y;
	out.m9 = y.z;
	out.m13 = -1.0f*center.y;

	out.m2 = z.x;
	out.m6 = z.y;
	out.m10 = z.z;
	out.m14 = -1.0f*center.z;

	out.m3 = 0.0f;
	out.m7 = 0.0f;
	out.m11 = 0.0f;
	out.m15 = 1.0f;

	return out;
}

unsigned int color(unsigned int r, unsigned int g, unsigned int b)
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
