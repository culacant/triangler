#include <unistd.h>

vec3f camray_model_intersect(model_raw m)
{
	vec3f o = CAMERA->pos;
	vec3f dir = vec3f_norm(vec3f_sub(CAMERA->target, CAMERA->pos));
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
	projectiles_init();

	char debug_text[256];
	model_raw iqe = loadiqe("res/testlvl.iqe");
	model_raw iqe_col = loadiqe("res/testlvl_col.iqe");
	model_raw iqe_sphere = loadiqe("res/sphere.iqe");
	texture t_tiles = loadtga("res/tiles.tga");

	model *m = load_model(iqe_col, iqe, &t_tiles);
	model *sphere = load_model(iqe_sphere, iqe_sphere, &t_tiles);
	sphere->draw = 0;

	player p = player_init((vec3f){10.0f, 10.0f, 1.0f});

	camera cam = {0};
	CAMERA = &cam;
	CAMERA->pos = (vec3f){-7.0f,15.0f,9.0f};
	CAMERA->angle = (vec2f){0.0f,0.0f};
	camera_target_from_angle(CAMERA);
	CAMERA->up = (vec3f){0.0f,0.0f,-1.0f};

	CAMERA->mv = mat_lookat(CAMERA->pos, CAMERA->target, CAMERA->up);
    CAMERA->proj = mat_identity();
	CAMERA->proj.m11 = -1.0f/vec3f_len(vec3f_sub(CAMERA->pos, CAMERA->target));
    CAMERA->vp = mat_viewport(RENDER_DATA.width/8, RENDER_DATA.height/8, RENDER_DATA.width*(16/9), RENDER_DATA.height*(16/9));

	camera_update_mat(CAMERA);

	while(!input_key(KEY_Q))
	{
		game_run(&p, m, sphere);
		game_flush();
		input_flush();

		CAMERA->pos = p.pos;
		CAMERA->angle = p.face;
		camera_target_from_angle(CAMERA);
		CAMERA->mv = mat_lookat(CAMERA->pos, CAMERA->target, CAMERA->up);
		camera_update_mat(CAMERA);

		render_run();
		sprintf(debug_text, "TIME: %i %i\npos: %f %f %f\nvel: %f %f %f\nflags: %i",
							GAME_DATA.frametime, RENDER_DATA.frametime,
							p.pos.x, p.pos.y, p.pos.z, 
							p.vel.x, p.vel.y, p.vel.z,p.flags);
		text_draw(5,5,debug_text , color_rgb(255,255,255));
		render_flush();
	}
	unloadtex(t_tiles);
	unload_model_raw(iqe);
	unload_model_raw(iqe_col);
	unload_model_raw(iqe_sphere);
	render_free();
	input_free();
	return 0;
}
