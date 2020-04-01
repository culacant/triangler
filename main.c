#include <unistd.h>

vec3f camray_model_intersect(model m)
{
	vec3f o = CAMERA->pos;
	vec3f dir = vec_norm(vec_sub(CAMERA->target, CAMERA->pos));
	intersection out = (intersection){0};
	out.distance = 999999.0f;
	for(int i=0;i<m.fcnt;i++)
	{
		vec3f a = m.vp[m.fm[i*3+0]];
		vec3f b = m.vp[m.fm[i*3+1]];
		vec3f c = m.vp[m.fm[i*3+2]];
		intersection cur = (intersection){0};
		if(ray_tri_intersect(o, dir, a, b, c, &cur))
		{
			if(cur.distance < out.distance)
				out = cur;
		}
	}
	return out.pos;
}

int main()
{
	char debug_text[256];
	texture t_head = loadtga("res/head.tga");
	model iqe = loadiqe("res/testlvl.iqe");
	model sphere = loadiqe("res/sphere.iqe");
	texture t_red = loadtga("res/red.tga");


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
    CAMERA->vp = mat_viewport(BUFFER.width/8, BUFFER.height/8, BUFFER.width*(16/9), BUFFER.height*(16/9));

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
			CAMERA->angle.x -= (float)(input_mouse_relx()*MOUSE_SENSITIVITY);
			CAMERA->angle.y += (float)(input_mouse_rely()*MOUSE_SENSITIVITY);
		}
		camera_target_from_angle(CAMERA);
		CAMERA->mv = mat_lookat(CAMERA->pos, CAMERA->target, CAMERA->up);
// length to target should be same since its normalized
//		CAMERA->proj.m11 = -1.0f/vec_len(vec_sub(CAMERA->pos, CAMERA->target));
		camera_update_mat(CAMERA);

		if(input_key(KEY_T))
		{
			sphere.trans = mat_transform(camray_model_intersect(iqe));
		}
		else
			sphere.trans = mat_identity();

		drawmodel_tex(iqe,t_head);
		drawmodel_tex(sphere,t_red);
//		drawmodel_wire(iqe, color_rgb(255,255,0));

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
	unloadtex(t_head);
	unloadmodel(iqe);
	unloadtex(t_red);
	unloadmodel(sphere);
	buf_free();
	input_free();
	return 0;
}
