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

#define GRAVITY (vec3f){0.0f, 0.0f, -0.001f}

int main()
{
	char debug_text[256];
	texture t_tiles = loadtga("res/tiles.tga");
	model iqe = loadiqe("res/testlvl.iqe");
	model iqe_col = loadiqe("res/testlvl_col.iqe");
	model sphere = loadiqe("res/sphere.iqe");
	texture t_red = loadtga("res/red.tga");

	player p = player_init((vec3f){10.0f, 10.0f, 1.0f});


	buf_init();
	input_init();

	camera cam = {0};
	CAMERA = &cam;
	CAMERA->pos = (vec3f){-7.0f,15.0f,9.0f};
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
		if(INPUTS.mouseactivity)
		{
			CAMERA->angle.x -= (float)(input_mouse_relx()*MOUSE_SENSITIVITY);
			CAMERA->angle.y += (float)(input_mouse_rely()*MOUSE_SENSITIVITY);
		}
		if(input_key(KEY_W))
			p.vel.x += 0.001f;
		if(input_key(KEY_S))
			p.vel.x -= 0.001f;
		if(input_key(KEY_A))
			p.vel.y += 0.001f;
		if(input_key(KEY_D))
			p.vel.y -= 0.001f;

		CAMERA->pos = p.pos;
		camera_target_from_angle(CAMERA);
		CAMERA->mv = mat_lookat(CAMERA->pos, CAMERA->target, CAMERA->up);
// length to target should be same since its normalized
//		CAMERA->proj.m11 = -1.0f/vec_len(vec_sub(CAMERA->pos, CAMERA->target));
		camera_update_mat(CAMERA);

		p.face = CAMERA->angle.x;
		player_vel_from_face(&p);
		player_collide(&p, iqe_col);
		p.vel = GRAVITY;
		player_collide(&p, iqe_col);
		p.pos.z += SMALLNR;

		sphere.trans = mat_transform(p.pos);
		drawmodel_tex(sphere, t_red);

		drawmodel_tex(iqe_col,t_tiles);
		sprintf(debug_text, "TIME: %i\npos: %f %f %f\nvel: %f %f %f\n",
							FRAMETIME,
							p.pos.x, p.pos.y, p.pos.z, 
							p.vel.x, p.vel.y, p.vel.z);
		text_draw(5,5,debug_text , color_rgb(255,255,255));

		if(input_key(KEY_Z))
			zbuf_to_tga("./zbuf.tga");
			
		buf_flush();
		input_flush();
	}
	unloadtex(t_tiles);
	unloadmodel(iqe);
	unloadmodel(iqe_col);
	unloadtex(t_red);
	unloadmodel(sphere);
	buf_free();
	input_free();
	return 0;
}
