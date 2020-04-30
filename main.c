#include <unistd.h>

vec3f camray_model_intersect(model_raw m)
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
	game_init();
	render_init();
	input_init();

	char debug_text[256];
	model_raw iqe = loadiqe("res/testlvl.iqe");
	model_raw iqe_col = loadiqe("res/testlvl_col.iqe");
	texture t_tiles = loadtga("res/tiles.tga");

	model *m = load_model(iqe_col, iqe, &t_tiles);

	player p = player_init((vec3f){10.0f, 10.0f, 1.0f});

	camera cam = {0};
	CAMERA = &cam;
	CAMERA->pos = (vec3f){-7.0f,15.0f,9.0f};
	CAMERA->angle = (vec2f){0.0f,0.0f};
	camera_target_from_angle(CAMERA);
	CAMERA->up = (vec3f){0.0f,0.0f,-1.0f};

	CAMERA->mv = mat_lookat(CAMERA->pos, CAMERA->target, CAMERA->up);
    CAMERA->proj = mat_identity();
	CAMERA->proj.m11 = -1.0f/vec_len(vec_sub(CAMERA->pos, CAMERA->target));
    CAMERA->vp = mat_viewport(RENDER_DATA.width/8, RENDER_DATA.height/8, RENDER_DATA.width*(16/9), RENDER_DATA.height*(16/9));

	camera_update_mat(CAMERA);

	while(!input_key(KEY_Q))
	{
		if(INPUT_DATA.mouseactivity)
		{
			CAMERA->angle.x -= (float)(input_mouse_relx()*MOUSE_SENSITIVITY);
			CAMERA->angle.y += (float)(input_mouse_rely()*MOUSE_SENSITIVITY);
		}
		if(input_key(KEY_W))
			p.impulse.x += 0.005f;
		if(input_key(KEY_S))
			p.impulse.x -= 0.005f;
		if(input_key(KEY_A))
			p.impulse.y += 0.005f;
		if(input_key(KEY_D))
			p.impulse.y -= 0.005f;
		if(input_key(KEY_SPACE))
			p.impulse.z += JUMP_HEIGHT;

// length to target should be same since its normalized
//		CAMERA->proj.m11 = -1.0f/vec_len(vec_sub(CAMERA->pos, CAMERA->target));
		p.face = CAMERA->angle.x;
		p.vel.z -= GRAVITY.z;
		player_vel_from_face(&p);
		player_collide(&p, m);



		if(input_key(KEY_Z))
			zbuf_to_tga("./zbuf.tga");
			
		game_flush();
		input_flush();

		CAMERA->pos = p.pos;
		camera_target_from_angle(CAMERA);
		CAMERA->mv = mat_lookat(CAMERA->pos, CAMERA->target, CAMERA->up);
		camera_update_mat(CAMERA);

		render_run();
		sprintf(debug_text, "TIME: %i\npos: %f %f %f\nvel: %f %f %f\nflags: %i",
							FRAMETIME,
							p.pos.x, p.pos.y, p.pos.z, 
							p.vel.x, p.vel.y, p.vel.z,p.flags);
		text_draw(5,5,debug_text , color_rgb(255,255,255));
		render_flush();
	}
	unloadtex(t_tiles);
	unload_model_raw(iqe);
	unload_model_raw(iqe_col);
	render_free();
	input_free();
	return 0;
}
