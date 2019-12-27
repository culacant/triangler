#include "render.h"
#include <unistd.h>

int main()
{
	char debug_text[256];
	texture t = loadtga("res/layout.tga");
	model iqe = loadiqe("res/cube.iqe");
	/*
	texture t2 = loadtga("res/head256.tga");
	model iqe2 = loadiqe("res/head.iqe");
	*/

	buf_init();
	input_init();

	camera cam = {0};
	CAMERA = &cam;

	CAMERA->pos = (vec3f){0.0f,13.0f,0.0f};
	CAMERA->angle = (vec2f){0.0f,0.0f};
	camera_target_from_angle(CAMERA);
	CAMERA->up = (vec3f){0.0f,0.0f,-1.0f};

	CAMERA->mv = mat_lookat(CAMERA->pos, CAMERA->target, CAMERA->up);
    CAMERA->proj = mat_identity();
	CAMERA->proj.m11 = -1.0f/vec_len(vec_sub(CAMERA->pos, CAMERA->target));
    CAMERA->vp = viewport(BUFFER.width/8, BUFFER.height/8, BUFFER.width*(16/9), BUFFER.height*(16/9));

	camera_update_mat(CAMERA);

	while(!input_key(KEY_Q))
	{
		if(input_key(KEY_W))
		{
			CAMERA->pos.z-=0.1f;
		}
		if(input_key(KEY_S))
		{
			CAMERA->pos.z+=0.1f;
		}
		if(input_key(KEY_A))
		{
			CAMERA->pos.x-=0.1f;
		}
		if(input_key(KEY_D))
		{
			CAMERA->pos.x+=0.1f;
		}
		if(input_key(KEY_R))
		{
			CAMERA->pos.y-=0.1f;
		}
		if(input_key(KEY_F))
		{
			CAMERA->pos.y+=0.1f;
		}
		if(input_key(KEY_C))
		{
			if(input_key(KEY_LEFTSHIFT))
				CAMERA->proj.m13 += 0.1f;
			else
				CAMERA->proj.m2 += 0.1f;
		}
		if(INPUTS.mouseactivity)
		{
			CAMERA->angle.x += (float)(input_mouse_relx()*MOUSE_SENSITIVITY);
			CAMERA->angle.y += (float)(input_mouse_rely()*MOUSE_SENSITIVITY);
		}
		camera_target_from_angle(CAMERA);
		CAMERA->mv = mat_lookat(CAMERA->pos, CAMERA->target, CAMERA->up);
// length to target should be same
//		CAMERA->proj.m11 = -1.0f/vec_len(vec_sub(CAMERA->pos, CAMERA->target));
		camera_update_mat(CAMERA);

		drawmodel_tex(iqe,t);
//		drawmodel_tex(iqe2,t2);
		drawmodel_wire(iqe, color_rgb(255,255,0));

		sprintf(debug_text, "TIME: %i\npos: %f %f %f\ntar: %f %f %f\nang: %f %f", 
							FRAMETIME,
							CAMERA->pos.x, CAMERA->pos.y, CAMERA->pos.z,
							CAMERA->target.x, CAMERA->target.y, CAMERA->target.z,
							CAMERA->angle.x, CAMERA->angle.y);
		text_draw(5,5,debug_text , color_rgb(255,255,255));

		if(input_key(KEY_Z))
			zbuf_to_tga("./zbuf.tga");
			
		buf_flush();
		input_flush();
	}
	unloadtex(t);
	unloadmodel(iqe);
//	unloadtex(t2);
//	unloadmodel(iqe2);
	buf_free();
	input_free();
	return 0;
}
