#include "render.h"
#include "font.h"
#include "mem.h"

void render_init()
{
	RENDER_MEM = mem_init(RENDER_MEM_SIZE);

	os_fb_init();

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

	os_fb_free();

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

            mat4f mv = mat_mul(RENDER_DATA.models[m].trans, CAMERA->view);
			mat4f mvn = mat_transpose(mat_invert(RENDER_DATA.models[m].trans));
            mat4f mvp = mat_mul(mv, CAMERA->proj);

            for(int f=0;f<RENDER_DATA.models[m].rtricnt;f++)
            {
				vec3f norm = vec3f_trans(RENDER_DATA.models[m].rtri[f].n, mvn);
				vec3f a = vec3f_trans(RENDER_DATA.models[m].rtri[f].a, RENDER_DATA.models[m].trans);
				vec3f normcam = vec3f_sub(a, CAMERA->pos);
				if(vec3f_dot(norm, normcam) > 0.f)
					continue;

				int incnt = 1;
				render_triangle in[CLIP_TRI_OUT];
				int outcnt = 0;
				render_triangle out[CLIP_TRI_OUT];

                in[0].a = vec3f_trans(RENDER_DATA.models[m].rtri[f].a, mv);
                in[0].b = vec3f_trans(RENDER_DATA.models[m].rtri[f].b, mv);
                in[0].c = vec3f_trans(RENDER_DATA.models[m].rtri[f].c, mv);
                in[0].uva = RENDER_DATA.models[m].rtri[f].uva;
                in[0].uvb = RENDER_DATA.models[m].rtri[f].uvb;
                in[0].uvc = RENDER_DATA.models[m].rtri[f].uvc;
                in[0].cola = RENDER_DATA.models[m].rtri[f].cola;
                in[0].colb = RENDER_DATA.models[m].rtri[f].colb;
                in[0].colc = RENDER_DATA.models[m].rtri[f].colc;
// clip z min
				vec3f p_z = (vec3f){0.f, 0.f, 0.1f};
				vec3f n_z = (vec3f){0.f, 0.f, 1.f};
				outcnt = triangle_clip_plane(p_z, n_z, in[0], &out[0],&out[1]);
				if(outcnt == 0)
					continue;
				incnt = outcnt;
				outcnt = 0;
				memcpy(in, out, sizeof(render_triangle)*incnt);
// transform
				for(int i=0;i<incnt;i++)
				{
					in[i].a = vec3f_trans(in[i].a, CAMERA->proj);
					in[i].b = vec3f_trans(in[i].b, CAMERA->proj);
					in[i].c = vec3f_trans(in[i].c, CAMERA->proj);
				}
// clip x min
				for(int i=0;i<incnt;i++)
				{
					vec3f p_minx = (vec3f){1.f, 0.f, 0.0f};
					vec3f n_minx = (vec3f){-1.f, 0.f, 0.f};
					outcnt += triangle_clip_plane(p_minx, n_minx, in[i], &out[outcnt],&out[outcnt+1]);
				}
				if(outcnt == 0)
					continue;
				incnt = outcnt;
				outcnt = 0;
				memcpy(in, out, sizeof(render_triangle)*incnt);
// clip x max
				for(int i=0;i<incnt;i++)
				{
					vec3f p_maxx = (vec3f){0.f, -1.f, 0.0f};
					vec3f n_maxx = (vec3f){0.f, 1.f, 0.f};
					outcnt += triangle_clip_plane(p_maxx, n_maxx, in[i], &out[outcnt],&out[outcnt+1]);
				}
				if(outcnt == 0)
					continue;
				incnt = outcnt;
				outcnt = 0;
				memcpy(in, out, sizeof(render_triangle)*incnt);
// clip y min
				for(int i=0;i<incnt;i++)
				{
					vec3f p_miny = (vec3f){0.f, 1.f, 0.0f};
					vec3f n_miny = (vec3f){0.f, -1.f, 0.f};
					outcnt += triangle_clip_plane(p_miny, n_miny, in[i], &out[outcnt],&out[outcnt+1]);
				}
				if(outcnt == 0)
					continue;
				incnt = outcnt;
				outcnt = 0;
				memcpy(in, out, sizeof(render_triangle)*incnt);
// clip y max
				for(int i=0;i<incnt;i++)
				{
					vec3f p_maxy = (vec3f){-1.f, 0.f, 0.0f};
					vec3f n_maxy = (vec3f){1.f, 0.f, 0.f};
					outcnt += triangle_clip_plane(p_maxy, n_maxy, in[i], &out[outcnt],&out[outcnt+1]);
				}
				if(outcnt == 0)
					continue;
// setup output
/*
				incnt = outcnt;
				outcnt = 0;
				memcpy(in, out, sizeof(render_triangle)*incnt);
*/
				vec3i ai;
				vec3i bi;
				vec3i ci;

                for(int i=0;i<outcnt;i++)
                {
					vec3f ofs = (vec3f){1,1,0};
					vec3f scale = (vec3f){0.5f*RENDER_DATA.width,0.5f*RENDER_DATA.height,ZBUF_DEPTH};

					out[i].a = vec3f_scale(out[i].a,-1.f);
					out[i].a = vec3f_add(out[i].a, ofs);
					out[i].a = vec3f_mul(out[i].a, scale);

					out[i].b = vec3f_scale(out[i].b,-1.f);
					out[i].b = vec3f_add(out[i].b, ofs);
					out[i].b = vec3f_mul(out[i].b, scale);

					out[i].c = vec3f_scale(out[i].c,-1.f);
					out[i].c = vec3f_add(out[i].c, ofs);
					out[i].c = vec3f_mul(out[i].c, scale);

                    ai = (vec3i){out[i].a.x, out[i].a.y, out[i].a.z};
                    bi = (vec3i){out[i].b.x, out[i].b.y, out[i].b.z};
                    ci = (vec3i){out[i].c.x, out[i].c.y, out[i].c.z};

/*
					vec3f cola = out[i].cola;
					vec3f colb = out[i].colb;
					vec3f colc = out[i].colc;
					triangle_color(ai,bi,ci,cola, colb, colc);
*/
					vec2f uva = out[i].uva;
					vec2f uvb = out[i].uvb;
					vec2f uvc = out[i].uvc;
					triangle_tex(ai,bi,ci,uva,uvb,uvc,1.0f,t);
                }
            }

        }
    }
}
void render_flush()
{
	os_fb_blit();
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

	curtime = os_timer_get();

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
	vec3f fwd = (vec3f){1.f, 0.f, 0.f};
	mat4f rmat = mat_rotate((vec3f){0.f, cam->face.y, cam->face.x});
	cam->target = vec3f_trans(fwd, rmat);

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

			stepzl = (float)(b.z-c.z)/(b.y-c.y);
			curzl = (float)b.z;

			stepcl.x = (float)(cb.x-cc.x)/(float)(b.y-c.y);
			stepcl.y = (float)(cb.y-cc.y)/(float)(b.y-c.y);
			stepcl.z = (float)(cb.z-cc.z)/(float)(b.y-c.y);
			curcl = cb;
		}

		for(int y=miny;y<maxy;y++)
		{   
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
				minz += dz;
				if(render_getz(x, y) < minz)
				{
					minc = vec3f_add(minc, dc);
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
	int depth = (a.z + b.z + c.z) / 3;
	for (int i = MIPMAP_CNT - 1; i >= 0; i--)
	{
		if (depth < MIPMAP_DEPTHS[i])
		{
			depth = i;
			break;
		}
	}

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
					uvi.x = minuv.x*t.width[depth];
					uvi.y = minuv.y*t.height[depth];
					unsigned int color = t.data[depth][uvi.x+uvi.y*t.width[depth]];

					render_px(x, y, color);
					render_setz(x, y, minz);
					if (x == RENDER_DATA.width / 2 && y == RENDER_DATA.height / 2)
						DEPTH = depth;
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
	for(int f=0;f<out.fcnt;f++)
	{
// TODO: sanitize blender export normals
/*
// calculate normals
		vec3f a = out.vp[out.fm[f*3+0]];
		vec3f b = out.vp[out.fm[f*3+1]];
		vec3f c = out.vp[out.fm[f*3+2]];

		out.fn[f] = vec3f_cross(vec3f_sub(b,a),vec3f_sub(c,a));
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
		out[f].cola = vec3f_scale(m.vc[fi],255.f);

		fi = m.fm[f*3+1];
		out[f].b = m.vp[fi];
		out[f].uvb = m.vt[fi];
		out[f].n = vec3f_add(out[f].n, m.vn[fi]);
		out[f].colb = vec3f_scale(m.vc[fi],255.f);

		fi = m.fm[f*3+2];
		out[f].c = m.vp[fi];
		out[f].uvc = m.vt[fi];
		out[f].n = vec3f_add(out[f].n, m.vn[fi]);
		out[f].colc = vec3f_scale(m.vc[fi],255.f);

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
	memcpy(&out.width[0], &header[12], sizeof(char)*2);
	memcpy(&out.height[0], &header[14], sizeof(char)*2);
	bbp = header[16];
// TODO: header sanity checking
	out.data[0] = calloc(sizeof(unsigned int),out.width[0]*out.height[0]);
	colordata = calloc(sizeof(unsigned char),out.width[0]*out.height[0]*bbp);
	if(fread(colordata, sizeof(unsigned char), out.width[0]*out.height[0]*bbp, fp) == 0)
	{
		printf("Can't read TGA color data from file %s\n", filename);
		return out;
	}
	int charcnt = 0;
	for(int i=0;i<out.width[0]*out.height[0];i++)
	{
		out.data[0][i] = colordata[charcnt++] << BSHIFT;
		out.data[0][i] += colordata[charcnt++] << GSHIFT;
		out.data[0][i] += colordata[charcnt++] << RSHIFT;
		out.data[0][i] += 0xff << ASHIFT;
		if(bbp == 32)
			charcnt++;
	}
	free(colordata);
	fclose(fp);

	texture_genmipmaps(&out);
	return out;
}
void unloadtex(texture t)
{
	for (int i = 0; i < MIPMAP_CNT; i++)
		free(t.data[i]);
}
void drawtex(texture t)
{
	for(int y=0; y<t.height[0];y++)
	{
		for(int x=0; x<t.width[0];x++)
		{
			render_px_safe(x,y,t.data[0][x+y*t.width[0]]);
		}
	}
}
void texture_genmipmaps(texture* t)
{
	for (int i = 1; i < MIPMAP_CNT; i++)
	{
		t->height[i] = t->height[i - 1] / 4;
		t->width[i] = t->width[i - 1] / 4;
		t->data[i] = calloc(sizeof(unsigned int), t->width[i] * t->height[i]);

		for (int y = 0; y < t->height[i]; y++)
		{
			for (int x = 0; x < t->width[i]; x++)
			{
				t->data[i][x + y * t->width[i]] = t->data[i - 1][(x * 4) + (y * 4) * t->width[i - 1]];
			}
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
		memcpy(posout, posin, sizeof(vec3f) * CLIP_TRI_IN);
		memcpy(uvout, uvin, sizeof(vec2f) * CLIP_TRI_IN);
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
int triangle_clip_plane(vec3f p, vec3f n, render_triangle in, render_triangle *out0,render_triangle *    out1)
{
    float dista = vec3f_dist_plane(p, n, in.a);
    float distb = vec3f_dist_plane(p, n, in.b);
    float distc = vec3f_dist_plane(p, n, in.c);

    if(dista < 0 && distb < 0 && distc < 0)
        return 0;
    else if(dista < 0)
    {   
        if(distb < 0)
        {
            float l1 = inv_lerp(dista, distc, 0.f);
            float l2 = inv_lerp(distb, distc, 0.f);
            out0->a = vec3f_lerp(in.a, in.c, l1);
            out0->b = vec3f_lerp(in.b, in.c, l2);
            out0->c = in.c;

            out0->cola = vec3f_lerp(in.cola, in.colc, l1);
            out0->colb = vec3f_lerp(in.colb, in.colc, l2);
            out0->colc = in.colc;

	        out0->uva = vec2f_lerp(in.uva, in.uvc, l1);
            out0->uvb = vec2f_lerp(in.uvb, in.uvc, l2);
            out0->uvc = in.uvc;
     
            return 1;
        }
        else if(distc < 0)
        {
            float l1 = inv_lerp(dista, distb, 0.f);
            float l2 = inv_lerp(distc, distb, 0.f);
            out0->a = vec3f_lerp(in.a, in.b, l1);
            out0->b = vec3f_lerp(in.c, in.b, l2);
            out0->c = in.b;

            out0->cola = vec3f_lerp(in.cola, in.colb, l1);
            out0->colb = vec3f_lerp(in.colc, in.colb, l2);
            out0->colc = in.colb;

            out0->uva = vec2f_lerp(in.uva, in.uvb, l1);
            out0->uvb = vec2f_lerp(in.uvc, in.uvb, l2);
            out0->uvc = in.uvb;
            return 1;
        }
        else
        {
            float l1 = inv_lerp(dista, distb, 0.f);
            float l2 = inv_lerp(dista, distc, 0.f);
            out0->a = vec3f_lerp(in.a, in.b, l1);
            out0->b = in.b;
            out0->c = in.c;

            out1->a = vec3f_lerp(in.a, in.c, l2);
            out1->b = out0->a;
            out1->c = in.c;

            out0->cola = vec3f_lerp(in.cola, in.colb, l1);
            out0->colb = in.colb;
            out0->colc = in.colc;

            out1->cola = vec3f_lerp(in.cola, in.colc, l2);
            out1->colb = out0->cola;
            out1->colc = in.colc;

            out0->uva = vec2f_lerp(in.uva, in.uvb, l1);
            out0->uvb = in.uvb;
            out0->uvc = in.uvc;

            out1->uva = vec2f_lerp(in.uva, in.uvc, l2);
            out1->uvb = out0->uva;
            out1->uvc = in.uvc;
 
            return 2;
        }
    }   
    else if(distb < 0)
    {   
        if(distc < 0)
        {
            float l1 = inv_lerp(distb, dista, 0.f);
            float l2 = inv_lerp(distc, dista, 0.f);
            out0->a = vec3f_lerp(in.b, in.a, l1);
            out0->b = vec3f_lerp(in.c, in.a, l2);
            out0->c = in.a;

            out0->cola = vec3f_lerp(in.colb, in.cola, l1);
            out0->colb = vec3f_lerp(in.colc, in.cola, l2);
            out0->colc = in.cola;

            out0->uva = vec2f_lerp(in.uvb, in.uva, l1);
            out0->uvb = vec2f_lerp(in.uvc, in.uva, l2);
            out0->uvc = in.uva;
            return 1;
        }
        else
        {
            float l1 = inv_lerp(distb, dista, 0.f);
            float l2 = inv_lerp(distb, distc, 0.f);
            out0->a = vec3f_lerp(in.b, in.a, l1);
            out0->b = in.a;
            out0->c = in.c;

            out1->a = vec3f_lerp(in.b, in.c, l2);
            out1->b = out0->a;
            out1->c = in.c;

            out0->cola = vec3f_lerp(in.colb, in.cola, l1);
            out0->colb = in.cola;
            out0->colc = in.colc;

            out1->cola = vec3f_lerp(in.colb, in.colc, l2);
            out1->colb = out0->cola;
            out1->colc = in.colc;

            out0->uva = vec2f_lerp(in.uvb, in.uva, l1);
            out0->uvb = in.uva;
            out0->uvc = in.uvc;

            out1->uva = vec2f_lerp(in.uvb, in.uvc, l2);
            out1->uvb = out0->uva;
            out1->uvc = in.uvc;

            return 2;
        }
    }
    else if(distc < 0)
    {
        float l1 = inv_lerp(distc, dista, 0.f);
        float l2 = inv_lerp(distc, distb, 0.f);
        out0->a = vec3f_lerp(in.c, in.a, l1);
        out0->b = in.a;
        out0->c = in.b;

        out1->a = vec3f_lerp(in.c, in.b, l2);
        out1->b = out0->a;
        out1->c = in.b;

        out0->cola = vec3f_lerp(in.colc, in.cola, l1);
        out0->colb = in.cola;
        out0->colc = in.colb;

        out1->cola = vec3f_lerp(in.colc, in.colb, l2);
        out1->colb = out0->cola;
        out1->colc = in.colb;

        out0->uva = vec2f_lerp(in.uvc, in.uva, l1);
        out0->uvb = in.uva;
        out0->uvc = in.uvb;

        out1->uva = vec2f_lerp(in.uvc, in.uvb, l2);
        out1->uvb = out0->uva;
        out1->uvc = in.uvb;
 
        return 2;
    }
    *out0 = in;
    return 1;

}

unsigned int color_rgb(unsigned char r, unsigned char g, unsigned char b)
{
	unsigned int color = r << RSHIFT;
	color |= g << GSHIFT;
	color |= b << BSHIFT;
	color |= 0xff << ASHIFT;
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

