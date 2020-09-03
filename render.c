#include "render.h"
#include "font.h"
#include "mem.h"

void render_init()
{
	RENDER_MEM = mem_init(RENDER_MEM_SIZE);
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

	RENDER_DATA.buf = calloc(RENDER_DATA.width*RENDER_DATA.height, sizeof(unsigned int));

	RENDER_DATA.zbuf = malloc(sizeof(int) * RENDER_DATA.width*RENDER_DATA.height);
	RENDER_DATA.zbufmin = malloc(sizeof(int) * RENDER_DATA.width*RENDER_DATA.height);

	for(int i=0;i<RENDER_DATA.width*RENDER_DATA.height;i++)
		RENDER_DATA.zbufmin[i] = ZBUF_MIN;

	memcpy(RENDER_DATA.zbuf, RENDER_DATA.zbufmin, sizeof(int)*RENDER_DATA.width*RENDER_DATA.height);

	RENDER_DATA.modelcnt = 0;
	RENDER_DATA.models = mem_alloc(sizeof(model)*MODEL_CNT, RENDER_MEM);
	RENDER_DATA.tricnt = 0;
	RENDER_DATA.tris = mem_alloc(sizeof(render_triangle)*TRIANGLE_CNT, RENDER_MEM);
}
void render_free()
{
	free(RENDER_DATA.buf);
	free(RENDER_DATA.zbuf);
	free(RENDER_DATA.zbufmin);
	munmap(RENDER_DATA.fb, 4*RENDER_DATA.width*RENDER_DATA.height);

	if(ioctl(RENDER_DATA.tty, KDSETMODE, KD_TEXT) == -1)
	{
		printf("WARNING: cannot set text mode on TTY: %s\n", TTY_NAME);
	}

	close(RENDER_DATA.tty);
	close(RENDER_DATA.fd);

	mem_free(RENDER_DATA.models);
	mem_free(RENDER_DATA.tris);
	free(RENDER_MEM);
}
void render_run()
{
    for(int m=0;m<MODEL_CNT;m++)
    {
        if(RENDER_DATA.models[m].flags & MODEL_FLAG_DRAW)
        {
            texture t = *RENDER_DATA.models[m].tex;

            int outcnt = 0;
            vec3f in[CLIP_POINT_IN];
            vec3f out[CLIP_POINT_OUT];
            vec2f uvin[CLIP_POINT_IN];
            vec2f uvout[CLIP_POINT_OUT];

			vec3f norm;
			vec3f transa;

            vec3i ai;
            vec3i bi;
            vec3i ci;

            vec2f uva;
            vec2f uvb;
            vec2f uvc;


            mat4f mv = mat_mul(RENDER_DATA.models[m].trans, CAMERA->view);

            for(int f=0;f<RENDER_DATA.models[m].rtricnt;f++)
            {
                in[0] = vec3f_trans(RENDER_DATA.models[m].rtri[f].a, mv);
                in[1] = vec3f_trans(RENDER_DATA.models[m].rtri[f].b, mv);
                in[2] = vec3f_trans(RENDER_DATA.models[m].rtri[f].c, mv);
                uvin[0] = RENDER_DATA.models[m].rtri[f].uva;
                uvin[1] = RENDER_DATA.models[m].rtri[f].uvb;
                uvin[2] = RENDER_DATA.models[m].rtri[f].uvc;

// FIXME: clean this up?
                norm = vec3f_norm(vec3f_trans(RENDER_DATA.models[m].rtri[f].n, RENDER_DATA.models[m].trans));
                transa = vec3f_trans(RENDER_DATA.models[m].rtri[f].a, RENDER_DATA.models[m].trans);
				vec3f normcam = vec3f_sub(transa, CAMERA->pos);
				if(vec3f_dot(norm, normcam) > 0.f)
					continue;

                triangle_clip_viewport(in, uvin, out, uvout, &outcnt);
                for(int i=0;i<outcnt;i++)
                {
                    out[i*3+0] = vec3f_trans(out[i*3+0], CAMERA->proj);
                    out[i*3+1] = vec3f_trans(out[i*3+1], CAMERA->proj);
                    out[i*3+2] = vec3f_trans(out[i*3+2], CAMERA->proj);

					vec3f ofs = (vec3f){1,1,0};
					vec3f scale = (vec3f){0.5f*RENDER_DATA.width,0.5f*RENDER_DATA.height,ZBUF_DEPTH};

					out[i*3+0] = vec3f_scale(out[i*3+0],-1.f);
					out[i*3+0] = vec3f_add(out[i*3+0], ofs);
					out[i*3+0] = vec3f_mul(out[i*3+0], scale);

					out[i*3+1] = vec3f_scale(out[i*3+1],-1.f);
					out[i*3+1] = vec3f_add(out[i*3+1], ofs);
					out[i*3+1] = vec3f_mul(out[i*3+1], scale);

					out[i*3+2] = vec3f_scale(out[i*3+2],-1.f);
					out[i*3+2] = vec3f_add(out[i*3+2], ofs);
					out[i*3+2] = vec3f_mul(out[i*3+2], scale);

                    ai = (vec3i){out[i*3+0].x, out[i*3+0].y, out[i*3+0].z};
                    bi = (vec3i){out[i*3+1].x, out[i*3+1].y, out[i*3+1].z};
                    ci = (vec3i){out[i*3+2].x, out[i*3+2].y, out[i*3+2].z};

                    uva = uvout[i*2+0];
                    uvb = uvout[i*2+1];
                    uvc = uvout[i*2+2];

					if(((ai.y == bi.y) && (ai.y == ci.y)) 	||
					   (ai.x<0 && bi.x<0 && ci.x<0)			||
					   (ai.y<0 && bi.y<0 && ci.y<0) 		||
					   (ai.x>=RENDER_DATA.width && bi.x>=RENDER_DATA.width && ci.x>=RENDER_DATA.width) ||
					   (ai.y>=RENDER_DATA.height&& bi.y>=RENDER_DATA.height&& ci.y>=RENDER_DATA.height))
						continue;
//					triangle_tex(ai,bi,ci,uva,uvb,uvc,1.0f,t);
					triangle_color(ai,bi,ci,(vec3f){255.f, 0.f, 0.f},(vec3f){0.f, 255.f, 0.f},(vec3f){0.f, 0.f, 255.f});
                }
            }

        }
    }
}
void render_flush()
{
	memcpy(RENDER_DATA.fb, RENDER_DATA.buf, sizeof(unsigned int)*RENDER_DATA.width*RENDER_DATA.height);
// pretty slow; zbuf flip?
	memset(RENDER_DATA.buf, 0, sizeof(unsigned int)*RENDER_DATA.width*RENDER_DATA.height);
	memcpy(RENDER_DATA.zbuf, RENDER_DATA.zbufmin, sizeof(int)*RENDER_DATA.width*RENDER_DATA.height);

}
void render_px(int x, int y, unsigned int color)
{
	RENDER_DATA.buf[y*RENDER_DATA.width+x] = color;
}
int render_getz(int x, int y)
{
	return RENDER_DATA.zbuf[y*RENDER_DATA.width+x];
}
void render_setz(int x, int y, int z)
{
	RENDER_DATA.zbuf[y*RENDER_DATA.width+x] = z;
}
void render_px_safe(int x, int y, unsigned int color)
{
	if(x<0 || y<0 || x>=RENDER_DATA.width || y>=RENDER_DATA.height)
		return;
	RENDER_DATA.buf[y*RENDER_DATA.width+x] = color;
}
int render_getz_safe(int x, int y)
{
	if(x<0 || y<0 || x>=RENDER_DATA.width || y>=RENDER_DATA.height)
		return ZBUF_MIN;
	else
		return RENDER_DATA.zbuf[y*RENDER_DATA.width+x];
}
void render_setz_safe(int x, int y, int z)
{
	if(x<0 || y<0 || x>=RENDER_DATA.width || y>=RENDER_DATA.height)
		return;
	RENDER_DATA.zbuf[y*RENDER_DATA.width+x] = z;
}

void zbuf_to_tga(const char *filename)
{
	char header[18] = {0};
	short width = (short)RENDER_DATA.width;
	short height = (short)RENDER_DATA.height;
	char *colordata = calloc(sizeof(unsigned char), RENDER_DATA.width*RENDER_DATA.height * 4);

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
	for(int i=0;i< RENDER_DATA.width*RENDER_DATA.height;i++)
	{
		colordata[cnt++] = (unsigned char)RENDER_DATA.zbuf[i];
		colordata[cnt++] = (unsigned char)RENDER_DATA.zbuf[i];
		colordata[cnt++] = (unsigned char)RENDER_DATA.zbuf[i];
		colordata[cnt++] = 255;
	}
	fwrite(header, sizeof(char), 18, fp);
	fwrite(colordata, sizeof(unsigned char), RENDER_DATA.width * RENDER_DATA.height * 4, fp);

	fclose(fp);
	free(colordata);
	
	return;
}

void render_frametime_update()
{
	static int lasttime;
	int curtime;
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	curtime = (int)(ts.tv_sec*1000 + ts.tv_nsec/1000000);
	RENDER_DATA.frametime = curtime - lasttime;
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
						render_px_safe(x+xi,y+yi, color);
					c >>= 1;
				}
			}
			x += TEXTRES;
		}
		i++;
	}
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
	cam->target.z = cosf(cam->angle.y);

	cam->target = vec3f_add(cam->target, cam->pos);
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
			render_px_safe(y,x,color);
		else
			render_px_safe(x,y,color);
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
				render_px_safe(y,x,color);
			else
				render_px_safe(x,y,color);
		}
		e2 += de2;
		if(e2 > dx)
		{
			y += (b.y>a.y?1:-1);
			e2 -= dx*2;
		}
	}
}
void triangle_color(vec3i a, vec3i b, vec3i c, vec3f ca, vec3f cb, vec3f cc)
{
	if(a.y>b.y)
	{
		vec3i_swap(&a, &b);
		vec3f_swap(&ca, &cb);
	}
	if(a.y>c.y)
	{
        vec3i_swap(&a, &c);
		vec3f_swap(&ca, &cc);
	}
    if(b.y>c.y)
	{
		vec3i_swap(&b, &c);
		vec3f_swap(&cb, &cc);
	}

	float stepxl;
	float curxl;
	float stepxs;
	float curxs;

	float stepzl;
	float curzl;
	float stepzs;
	float curzs;

	vec3f stepcl;
	vec3f curcl;
	vec3f stepcs;
	vec3f curcs;

	int miny;
	int maxy;

	for(int h=0;h<2;h++)
	{
		if(h==0)
		{
			miny = a.y;
			maxy = b.y;

			stepxl = (float)(a.x-b.x)/(float)(a.y-b.y);
			curxl = (float)a.x;
			stepxs = (float)(a.x-c.x)/(float)(a.y-c.y);
			curxs = (float)a.x;

			stepzl = (float)(a.z-b.z)/(float)(a.y-b.y);
			curzl = (float)a.z;
			stepzs = (float)(a.z-c.z)/(float)(a.y-c.y);
			curzs = (float)a.z;

			stepcl.x = (float)(ca.x-cb.x)/(float)(a.y-b.y);
			stepcl.y = (float)(ca.y-cb.y)/(float)(a.y-b.y);
			stepcl.z = (float)(ca.z-cb.z)/(float)(a.y-b.y);
			curcl = ca;
			stepcs.x = (float)(ca.x-cc.x)/(float)(a.y-c.y);
			stepcs.y = (float)(ca.y-cc.y)/(float)(a.y-c.y);
			stepcs.z = (float)(ca.z-cc.z)/(float)(a.y-c.y);
			curcs = ca;
		}
		else
		{
			miny = b.y;
			maxy = c.y;

			stepxl = (float)(b.x-c.x)/(float)(b.y-c.y);
			curxl = (float)b.x;
			curxs -= stepxs;

			stepzl = (float)(b.z-c.z)/(b.y-c.y);
			curzl = (float)b.z;
			curzs -= stepzs;

			stepcl.x = (float)(cb.x-cc.x)/(float)(b.y-c.y);
			stepcl.y = (float)(cb.y-cc.y)/(float)(b.y-c.y);
			stepcl.z = (float)(cb.z-cc.z)/(float)(b.y-c.y);
			curcl = cb;
			curcs = vec3f_sub(curcs, stepcs);
		}

		for(int y=miny;y<maxy;y++)
		{   
			if(y<0)
			{
				curxl += stepxl;
				curxs += stepxs;

				curzl += stepzl;
				curzs += stepzs;

				curcl = vec3f_add(curcl, stepcl);
				curcs = vec3f_add(curcs, stepcs);
				continue;
			}
			else if(y >= RENDER_DATA.height)
				break;

			int minx = (int)curxl;
			int maxx = (int)curxs;
			float minz = curzl;
			float maxz = curzs;

			vec3f minc = curcl;
			vec3f maxc = curcs;

			if(minx>maxx)
			{
				int tmp = minx;
				minx = maxx;
				maxx = tmp;
				float tmpf = minz;
				minz = maxz;
				maxz = tmpf;

				vec3f_swap(&minc, &maxc);
			}

			float dz = (maxz-minz)/(float)(maxx-minx);

			vec3f dc;
			dc.x = (maxc.x-minc.x)/(float)(maxx-minx);
			dc.y = (maxc.y-minc.y)/(float)(maxx-minx);
			dc.z = (maxc.z-minc.z)/(float)(maxx-minx);

			for(int x = minx;x<maxx;x++)
			{
				if(x<0)
					continue;
				else if(x >= RENDER_DATA.width)
					break;

				minz += dz;

				minc = vec3f_add(minc, dc);

				if(render_getz(x, y) < minz)
				{
					unsigned int col = color_rgb(minc.x, minc.y, minc.z);
					render_px(x, y, col);
					render_setz(x, y, minz);
				}
			}

			curxl += stepxl;
			curxs += stepxs;
			curzl += stepzl;
			curzs += stepzs;

			curcl = vec3f_add(curcl, stepcl);
			curcs = vec3f_add(curcs, stepcs);
		}
	}
}
void triangle_tex(vec3i a, vec3i b, vec3i c, vec2f uva, vec2f uvb, vec2f uvc, float bright, texture t)
{
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

	float stepxl;
	float curxl;
	float stepxs;
	float curxs;

	float stepzl;
	float curzl;
	float stepzs;
	float curzs;

	vec2f stepuvl;
	vec2f curuvl;
	vec2f stepuvs;
	vec2f curuvs;


	int miny;
	int maxy;

	for(int h=0;h<2;h++)
	{
		if(h==0)
		{
			miny = a.y;
			maxy = b.y;

			stepxl = (float)(a.x-b.x)/(float)(a.y-b.y);
			curxl = (float)a.x;
			stepxs = (float)(a.x-c.x)/(float)(a.y-c.y);
			curxs = (float)a.x;

			stepzl = (float)(a.z-b.z)/(float)(a.y-b.y);
			curzl = (float)a.z;
			stepzs = (float)(a.z-c.z)/(float)(a.y-c.y);
			curzs = (float)a.z;

			stepuvl.x = (float)(uva.x-uvb.x)/(float)(a.y-b.y);
			stepuvl.y = (float)(uva.y-uvb.y)/(float)(a.y-b.y);
			curuvl = uva;
			stepuvs.x = (float)(uva.x-uvc.x)/(float)(a.y-c.y);
			stepuvs.y = (float)(uva.y-uvc.y)/(float)(a.y-c.y);
			curuvs = uva;

		}
		else
		{
			miny = b.y;
			maxy = c.y;

			stepxl = (float)(b.x-c.x)/(float)(b.y-c.y);
			curxl = (float)b.x;
			curxs -= stepxs;

			stepzl = (float)(b.z-c.z)/(b.y-c.y);
			curzl = (float)b.z;
			curzs -= stepzs;

			stepuvl.x = (float)(uvb.x-uvc.x)/(float)(b.y-c.y);
			stepuvl.y = (float)(uvb.y-uvc.y)/(float)(b.y-c.y);
			curuvl = uvb;
		}

		for(int y=miny;y<maxy;y++)
		{   
			if(y<0)
			{
				curxl += stepxl;
				curxs += stepxs;

				curzl += stepzl;
				curzs += stepzs;

				curuvl = vec2f_add(curuvl, stepuvl);
				curuvs = vec2f_add(curuvs, stepuvs);
				continue;
			}
			else if(y >= RENDER_DATA.height)
				break;

			int minx = (int)curxl;
			int maxx = (int)curxs;
			float minz = curzl;
			float maxz = curzs;
			vec2f minuv = curuvl;
			vec2f maxuv = curuvs;

			if(minx>maxx)
			{
				int tmp = minx;
				minx = maxx;
				maxx = tmp;
				float tmpf = minz;
				minz = maxz;
				maxz = tmpf;
				vec2f_swap(&minuv, &maxuv);
			}

			float dz = (maxz-minz)/(float)(maxx-minx);
			vec2f duv;
			duv.x = (maxuv.x-minuv.x)/(float)(maxx-minx);
			duv.y = (maxuv.y-minuv.y)/(float)(maxx-minx);

			for(int x = minx;x<maxx;x++)
			{
				if(x<0)
					continue;
				else if(x >= RENDER_DATA.width)
					break;

				minz += dz;
				minuv = vec2f_add(minuv, duv);
				if(render_getz(x, y) < minz)
				{
					vec2i uvi;
					uvi.x = minuv.x*t.width;
					uvi.y = minuv.y*t.height;
					unsigned int color = t.data[uvi.x+uvi.y*t.width];

					render_px(x, y, color);
					render_setz(x, y, minz);
				}
			}

			curxl += stepxl;
			curxs += stepxs;
			curzl += stepzl;
			curzs += stepzs;
			curuvl = vec2f_add(curuvl, stepuvl);
			curuvs = vec2f_add(curuvs, stepuvs);
		}
	}
}

void rect(vec2i a, vec2i b, unsigned int color)
{
	for(int y=a.y;y<a.y+b.y;y++)
	{
		for(int x=a.x;x<a.x+b.x;x++)
		{
			render_px_safe(x,y,color);
		}
	}
}

model_raw loadiqe(const char *filename)
{
	char line[512];
	model_raw out = {0};
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
	out.vc = malloc(sizeof(vec3f)*out.vcnt);
	out.fm = malloc(sizeof(int)*out.fcnt*3);
	out.fn = malloc(sizeof(vec3f)*out.fcnt);
	rewind(fp);
// color initial
	for(int i=0;i<out.vcnt;i++)
		out.vc[i] = (vec3f){-1.f, -1.f, -1.f};
// second pass
	int vpcnt = 0;
	int vtcnt = 0;
	int vncnt = 0;
	int vccnt = 0;
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
		else if(sscanf(line," vc %f %f %f", &invec.x, &invec.y, &invec.z) == 3)
		{
			out.vc[vccnt] = invec;
			vccnt++;
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
/*
	vec3f a;
	vec3f b;
	vec3f c;
*/
	for(int f=0;f<out.fcnt;f++)
	{
// TODO: sanitize blender export normals
/*
		a = out.vp[out.fm[f*3+0]];
		b = out.vp[out.fm[f*3+1]];
		c = out.vp[out.fm[f*3+2]];

		out.fn[f] = vec_cross(vec3f_sub(b,a),vec3f_sub(c,a));
		out.fn[f] = vec3f_norm(out.fn[f]);
*/

		out.fn[f] = vec3f_norm(out.vn[out.fm[f*3+0]]);
	}
	fclose(fp);
	return out;
}
void unload_model_raw(model_raw m)
{
	free(m.vp);
	free(m.vt);
	free(m.vn);
	free(m.vc);
	free(m.fm);
	free(m.fn);
}

render_triangle* render_load_tris(model_raw m)
{
	render_triangle *out  = malloc_render_tri(m.fcnt);
	int fi;
	for(int f=0;f<m.fcnt;f++)
	{
		fi = m.fm[f*3+0];
		out[f].a = m.vp[fi];
		out[f].uva = m.vt[fi];
		out[f].n = m.vn[fi];
		out[f].cola = m.vc[fi];

		fi = m.fm[f*3+1];
		out[f].b = m.vp[fi];
		out[f].uvb = m.vt[fi];
		out[f].n = vec3f_add(out[f].n, m.vn[fi]);
		out[f].colb = m.vc[fi];

		fi = m.fm[f*3+2];
		out[f].c = m.vp[fi];
		out[f].uvc = m.vt[fi];
		out[f].n = vec3f_add(out[f].n, m.vn[fi]);
		out[f].colc = m.vc[fi];

		out[f].n = vec3f_norm(out[f].n);
	}
	return out;
}
game_triangle* game_load_tris(model_raw m)
{
	game_triangle *out  = malloc_game_tri(m.fcnt);
	int fi;
	for(int f=0;f<m.fcnt;f++)
	{
		fi = m.fm[f*3+0];
		out[f].a = m.vp[fi];
		out[f].n = m.vn[fi];

		fi = m.fm[f*3+1];
		out[f].b = m.vp[fi];
		out[f].n = vec3f_add(out[f].n, m.vn[fi]);

		fi = m.fm[f*3+2];
		out[f].c = m.vp[fi];
		out[f].n = vec3f_add(out[f].n, m.vn[fi]);

		out[f].n = vec3f_norm(out[f].n);
	}
	return out;
}
model* load_model(model_raw gmodel, model_raw rmodel, texture *t)
{
	model *out = malloc_model(1);

	out->flags = MODEL_FLAG_DRAW;
	out->rtricnt = rmodel.fcnt;
	out->rtri = render_load_tris(rmodel);
	out->gtricnt = gmodel.fcnt;
	out->gtri = game_load_tris(gmodel);
	out->tex = t;
	out->trans = mat_identity();

	return out;
}
model* dupe_model(model *m)
{
	model *out = malloc_model(1);
	if(!out)
		return NULL;
	memcpy(out, m, sizeof(model));
	return out;
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
			render_px_safe(x,y,t.data[x+y*t.width]);
		}
	}
}

void triangle_clip_viewport(vec3f *posin, vec2f *uvin, vec3f *posout, vec2f *uvout, int *cntout)
{
	if(posin[0].z <= CLIP_NEAR && posin[1].z <= CLIP_NEAR && posin[2].z <= CLIP_NEAR)
	{
		*cntout = 0;
		return;
	}
	else if(posin[0].z <= CLIP_NEAR)
	{
		if(posin[1].z <= CLIP_NEAR)
		{
			*cntout = 1;
			triangle_clip_double(posin[2], posin[0], posin[1], uvin[2], uvin[0], uvin[1], posout, uvout);
			return;
		}
		else if(posin[2].z <= CLIP_NEAR)
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
	else if(posin[1].z <= CLIP_NEAR)
	{
		if(posin[2].z <= CLIP_NEAR)
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
	else if(posin[2].z <= CLIP_NEAR)
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
}
void triangle_clip_double(vec3f in, vec3f out1, vec3f out2, vec2f inuv, vec2f out1uv, vec2f out2uv, vec3f *posout, vec2f *uvout)
{
	float lerp1 = inv_lerp(out1.z, in.z, CLIP_NEAR);
	float lerp2 = inv_lerp(out2.z, in.z, CLIP_NEAR);

	posout[0] = vec3f_lerp(out1, in, lerp1);
	posout[1] = vec3f_lerp(out2, in, lerp2);
	posout[2] = in;

	uvout[0] = vec2f_lerp(out1uv, inuv, lerp1);
	uvout[1] = vec2f_lerp(out2uv, inuv, lerp2);
	uvout[2] = inuv;
}

unsigned int color_rgb(unsigned char r, unsigned char g, unsigned char b)
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
	a = c & BMASK;
	out = (int)(a*b) << BSHIFT;
	a = (c & GMASK) >> GSHIFT;
	out += (int)(a*b) << GSHIFT;
	a = (c & RMASK) >> RSHIFT;
	out += (int)(a*b) << RSHIFT;
	return out;
}

void print_mat(mat4f m)
{
    printf("MAT4F:\n");
	printf("%f %f %f %f\n", m.m0, m.m4, m.m8, m.m12);
	printf("%f %f %f %f\n", m.m1, m.m5, m.m9, m.m13);
	printf("%f %f %f %f\n", m.m2, m.m6, m.m10, m.m14);
	printf("%f %f %f %f\n", m.m3, m.m7, m.m11, m.m15);
}
void print_vec3f( vec3f v)
{
	printf("VEC3F:\n");
	printf("%f %f %f\n", v.x, v.y, v.z);
}
void print_vec3i( vec3i v)
{
   printf("VEC3I:\n");
   printf("%i %i %i\n", v.x, v.y, v.z);
}
void print_vec4f( vec4f v)
{
   printf("VEC4F:\n");
   printf("%f %f %f %f\n", v.x, v.y, v.z, v.w);
}

