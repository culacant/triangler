#include "render.h"
#include <unistd.h>

int main()
{
/*
	texture t = loadtga("res/uv.tga");
	model iqe = loadiqe("res/cube.iqe");
*/
	texture t = loadtga("res/head256.tga");
	model iqe = loadiqe("res/head.iqe");

	buf_init();

	vec3f eye = (vec3f){1.0f,0.5f,-5.0f};
	vec3f center = (vec3f){0.1f,0.1f,0.1f};
	mat4f mv = mat_lookat(eye,center,(vec3f){0.0f,-1.0f,0.0f});
    mat4f proj = identity();
    proj.m11 = -1.0f/(vec_norm(vec_sub(eye,center)).z);
    mat4f vp = viewport(BUFFER.width/8, BUFFER.height/8, BUFFER.width*3/4, BUFFER.height*3/4);

	update_mv(mv);
	update_vp(vp);
	update_proj(proj);

	drawmodel_tex(iqe,t);
	buf_flush();

	for(int i=0;i<1000;i++)
	{
		eye.x = (rand()%40)-20;
		eye.y = (rand()%40)-20;
		eye.z = (rand()%40)-20;
		mv = mat_lookat(eye,center,(vec3f){0.0f,-1.0f,0.0f});
		update_mv(mv);
		drawmodel_tex(iqe,t);
		buf_flush();
	}
/*
	for(int i=0;i<10;i++)
	{
		eye.x = i;
		mv = mat_lookat(eye,center,(vec3f){0.0f,-1.0f,0.0f});
		update_mv(mv);
		drawmodel_tex(iqe,t);
		buf_flush();
	}
	eye = (vec3f){1.0f,1.0f,3.0f};
	for(int i=0;i<10;i++)
	{
		eye.y = i;
		mv = mat_lookat(eye,center,(vec3f){0.0f,-1.0f,0.0f});
		update_mv(mv);
		drawmodel_tex(iqe,t);
		buf_flush();
	}
	eye = (vec3f){1.0f,1.0f,3.0f};
	for(int i=0;i<10;i++)
	{
		eye.z = 3.0f+i;
		mv = mat_lookat(eye,center,(vec3f){0.0f,-1.0f,0.0f});
		update_mv(mv);
		drawmodel_tex(iqe,t);
		buf_flush();
	}
*/
	unloadtex(t);
	unloadmodel(iqe);
	buf_free();
	return 0;
}
