#include "thread.h"

render_triangle* rbuff_load_model(int vcnt, vec3f *vp, vec2f *vt, vec3f *vn, int fcnt, int *fm)
{
	render_triangle *out = malloc_rbuff_tri(fcnt);
	for(int f=0;f<fcnt;f++)
	{
		out[f].a = vp[f*3+0];
		out[f].uva = vt[f*3+0];
		out[f].n = vn[f*3+0];

		out[f].b = vp[f*3+1];
		out[f].uvb = vt[f*3+1];
		out[f].n = vec3f_add(out[f].n, vn[f*3+1]);

		out[f].c = vp[f*3+2];
		out[f].uvc = vt[f*3+2];
		out[f].n = vec3f_add(out[f].n, vn[f*3+2]);

		out[f].n = vec3f_norm(out[f].n);
	}
	return out;
}
game_triangle* gbuff_load_model(int vcnt, vec3f *vp, vec3f *vn, int fcnt, int *fm)
{
	game_triangle *out = malloc_gbuff_tri(fcnt);
	for(int f=0;f<fcnt;f++)
	{
		out[f].a = vp[f*3+0];
		out[f].n = vn[f*3+0];

		out[f].b = vp[f*3+1];
		out[f].n = vec3f_add(out[f].n, vn[f*3+1]);

		out[f].c = vp[f*3+2];
		out[f].n = vec3f_add(out[f].n, vn[f*3+2]);

		out[f].n = vec3f_norm(out[f].n);
	}
	return out;
}

model *buff_load_model(model_raw m, texture *t)
{
	model *out = malloc_mbuff(1);
	
	out->draw = 0;
	out->cnt_rtri = m.fcnt;
	out->rtri = rbuff_load_model(m.vcnt, m.vp, m.vt, m.vn, m.fcnt, m.fm);
	out->cnt_gtri = m.fcnt;
	out->gtri = gbuff_load_model(m.vcnt, m.vp, m.vn, m.fcnt, m.fm);
	out->tex = t;
	out->trans = mat_identity();

	return out;
}

void render_rbuff()
{
	for(int m=0;m<RENDER_BUFFERS.mbuff_cnt;m++)
	{
		if(RENDER_BUFFERS.mbuff[m].draw)
		{
			texture t = *RENDER_BUFFERS.mbuff[m].tex;

			int outcnt = 0;
			vec3f in[CLIP_POINT_IN];
			vec3f out[CLIP_POINT_OUT];
			vec2f uvin[CLIP_POINT_IN];
			vec2f uvout[CLIP_POINT_OUT];

			vec3i ai;
			vec3i bi;
			vec3i ci;

			vec2f uva;
			vec2f uvb;
			vec2f uvc;

			mat4f mv = mat_mul(RENDER_BUFFERS.mbuff[m].trans, CAMERA->mv);

			for(int f=0;f<RENDER_BUFFERS.mbuff[m].cnt_rtri;f++)
			{
				in[0] = vec3f_trans(RENDER_BUFFERS.mbuff[m].rtri[f].a, mv);
				in[1] = vec3f_trans(RENDER_BUFFERS.mbuff[m].rtri[f].b, mv);
				in[2] = vec3f_trans(RENDER_BUFFERS.mbuff[m].rtri[f].c, mv);
		// FIXME: normals
				uvin[0] = RENDER_BUFFERS.mbuff[m].rtri[f].uva;
				uvin[1] = RENDER_BUFFERS.mbuff[m].rtri[f].uvb;
				uvin[2] = RENDER_BUFFERS.mbuff[m].rtri[f].uvc;

				triangle_clip_viewport(in, uvin, out, uvout, &outcnt);
				for(int i=0;i<outcnt;i++)
				{
					out[i*3+0] = vec3f_trans(out[i*3+0], CAMERA->fin);
					out[i*3+1] = vec3f_trans(out[i*3+1], CAMERA->fin);
					out[i*3+2] = vec3f_trans(out[i*3+2], CAMERA->fin);

					ai = (vec3i){out[i*3+0].x, out[i*3+0].y, out[i*3+0].z};
					bi = (vec3i){out[i*3+1].x, out[i*3+1].y, out[i*3+1].z};
					ci = (vec3i){out[i*3+2].x, out[i*3+2].y, out[i*3+2].z};

					uva = uvout[i*2+0];
					uvb = uvout[i*2+1];
					uvc = uvout[i*2+2];

					triangle_tex(ai,bi,ci,uva,uvb,uvc,1.0f,t);

				}
			}
			
		}
	}
}

void* malloc_rbuff_tri(int cnt)
{
	void *addr = &RENDER_BUFFERS.tbuff[GAME_BUFFERS.tbuff_cnt];
	RENDER_BUFFERS.tbuff_cnt += cnt;
	return addr;
}
void* malloc_gbuff_tri(int cnt)
{
	void *addr = &GAME_BUFFERS.tbuff[GAME_BUFFERS.tbuff_cnt];
	GAME_BUFFERS.tbuff_cnt += cnt;
	return addr;
}
void* malloc_mbuff(int cnt)
{
	void *addr = &RENDER_BUFFERS.mbuff[RENDER_BUFFERS.mbuff_cnt];
	RENDER_BUFFERS.mbuff_cnt += cnt;
	return addr;
}
