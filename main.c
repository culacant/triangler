#include "render.h"
#include <unistd.h>

int main()
{
	char text[25];
//	texture t = loadtga("res/layout.tga");
//	model iqe = loadiqe("res/cube.iqe");
	texture t = loadtga("res/head256.tga");
	model iqe = loadiqe("res/head.iqe");

	buf_init();
	input_init();

	camera cam = {0};
	CAMERA = &cam;

	CAMERA->pos = (vec3f){0.0f,0.0f,3.0f};
	CAMERA->target = (vec3f){0.0f,0.0f,0.0f};
	CAMERA->up = (vec3f){0.0f,-1.0f,0.0f};
/*
	camera_angle_from_target(CAMERA);
	CAMERA->angle = (vec2f){0.01f, 0.01f};
*/
//	camera_angle_from_target(CAMERA);

	CAMERA->mv = mat_lookat(CAMERA->pos, CAMERA->target, CAMERA->up);
    CAMERA->proj = mat_identity();
    CAMERA->proj.m11 = 1.0f/vec_len(vec_sub(CAMERA->pos, CAMERA->target));
    CAMERA->vp = viewport(BUFFER.width/8, BUFFER.height/8, BUFFER.width*(16/9), BUFFER.height*(16/9));
	camera_update_mat(CAMERA);

	while(!input_key(KEY_Q))
	{
		if(input_key(KEY_W))
		{
			if(input_key(KEY_LEFTSHIFT))
				CAMERA->target.x+=0.1f;
			else
				CAMERA->pos.x+=1.0f;
		}
		if(input_key(KEY_S))
		{
			if(input_key(KEY_LEFTSHIFT))
				CAMERA->target.x-=0.1f;
			else
				CAMERA->pos.x-=1.0f;
		}
		if(input_key(KEY_A))
		{
			if(input_key(KEY_LEFTSHIFT))
				CAMERA->target.y+=0.1f;
			else
				CAMERA->pos.y+=1.0f;
		}
		if(input_key(KEY_D))
		{
			if(input_key(KEY_LEFTSHIFT))
				CAMERA->target.y-=0.1f;
			else
				CAMERA->pos.y-=1.0f;
		}
		if(input_key(KEY_R))
		{
			if(input_key(KEY_LEFTSHIFT))
				CAMERA->target.z+=0.1f;
			else
				CAMERA->pos.z+=1.0f;
		}
		if(input_key(KEY_F))
		{
			if(input_key(KEY_LEFTSHIFT))
				CAMERA->target.z-=0.1f;
			else
				CAMERA->pos.z-=1.0f;
		}
		if(input_key(KEY_C))
		{
//			printf("angle %f %f\n", CAMERA->angle.x, CAMERA->angle.y);
			if(input_key(KEY_LEFTSHIFT))
				CAMERA->proj.m13 += 0.1f;
			else
				CAMERA->proj.m2 += 0.1f;
		}
		if(INPUTS.mouseactivity)
		{
			CAMERA->angle.x += (float)(input_mouse_relx()*MOUSE_SENSITIVITY);
			CAMERA->angle.y += (float)(input_mouse_rely()*MOUSE_SENSITIVITY);
//			camera_target_from_angle(CAMERA);
		}

		CAMERA->mv = mat_lookat(CAMERA->pos, CAMERA->target, CAMERA->up);
		CAMERA->proj.m11 = 1.0f/vec_len(vec_sub(CAMERA->pos, CAMERA->target));
		camera_update_mat(CAMERA);

		drawmodel_tex(iqe,t);

		sprintf(text, "%i", FRAMETIME);
		text_draw(5,5,text , color(255,255,255));
		buf_flush();
		input_flush();
	}

	unloadtex(t);
	unloadmodel(iqe);
	buf_free();
	input_free();
	return 0;
}
