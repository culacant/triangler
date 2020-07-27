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

	char debug_text[2048];
	model_raw iqe_sphere = loadiqe("res/sphere.iqe");
	model_raw iqe_knight = loadiqe("res/knight.iqe");
	texture t_tiles = loadtga("res/tiles.tga");

	model_raw iqe_wall = loadiqe("res/lvl/level1_wall.iqe");
	model_raw iqe_floor = loadiqe("res/lvl/level1_floor.iqe");
	
	model_raw iqe_wall_col = loadiqe("res/lvl/level1_wall_col.iqe");
	model_raw iqe_floor_col = loadiqe("res/lvl/level1_floor_col.iqe");

	texture t_floor = loadtga("res/lvl/floor.tga");
	texture t_wall = loadtga("res/lvl/wall.tga");

//	model *m = load_model(iqe_col, iqe, &t_tiles);

	model *m = load_model(iqe_floor, iqe_floor, &t_floor);
	m->flags = MODEL_FLAG_COLLIDE | MODEL_FLAG_DRAW;
	model *m2 = load_model(iqe_wall, iqe_wall, &t_wall);
	m2->flags = MODEL_FLAG_COLLIDE | MODEL_FLAG_DRAW;

	model *sphere = load_model(iqe_sphere, iqe_sphere, &t_tiles);
	sphere->flags = 0;
	model *knight = load_model(iqe_knight, iqe_knight, &t_tiles);
	knight->flags = 0;

	unload_model_raw(iqe_wall);
	unload_model_raw(iqe_floor);
	unload_model_raw(iqe_wall_col);
	unload_model_raw(iqe_floor_col);
/*
	unload_model_raw(iqe);
	unload_model_raw(iqe_col);
*/
	unload_model_raw(iqe_sphere);
	unload_model_raw(iqe_knight);

	player p = player_init((vec3f){35.0f, 0.0f, 30.0f});

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

	int rtime = 0;
	int gtime = 0;
/*
print_mem(GAME_MEM, 0);
return 0;
*/

	while(!input_key(KEY_Q))
	{
// game start
		game_frametime_update();

		input_flush();
		game_run(&p, m, sphere);
		game_flush();

		game_frametime_update();
// game end
// render start
		render_frametime_update();

		CAMERA->pos = p.pos;
		CAMERA->angle = p.face;
		camera_target_from_angle(CAMERA);
		CAMERA->mv = mat_lookat(CAMERA->pos, CAMERA->target, CAMERA->up);
		camera_update_mat(CAMERA);

		render_run();
//		render_flush();

// debug start
		sprintf(debug_text, "TIME: %i %i\npos: %f %f %f\nvel: %f %f %f | %f\nflags: %i",
							gtime, rtime,
							p.pos.x, p.pos.y, p.pos.z, 
							p.vel.x, p.vel.y, p.vel.z, vec3f_len(p.vel),
							p.flags);
		char models_text[1024];
		print_models(models_text);
		strcat(debug_text, models_text);
		print_projectiles(models_text);
		strcat(debug_text, models_text);

		text_draw(5,5,debug_text , color_rgb(255,255,255));
		render_flush();
		render_frametime_update();
// render end

		gtime = GAME_DATA.frametime;
		rtime = RENDER_DATA.frametime;
// debug end
	}
	unloadtex(t_tiles);
	render_free();
	input_free();
	return 0;
}
