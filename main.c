
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
	texture t_tiles = loadtga("res/tiles.tga");
	texture t_floor = loadtga("res/lvl/floor.tga");
	texture t_wall = loadtga("res/lvl/wall.tga");

	model_raw iqe_sphere = loadiqe("res/sphere.iqe");
	model_raw iqe_knight = loadiqe("res/knight.iqe");

	model_raw iqe_tri = loadiqe("res/tri.iqe");
	model *tri = load_model(iqe_tri, iqe_tri, &t_tiles);
	tri->flags = MODEL_FLAG_DRAW;
	unload_model_raw(iqe_tri);

	model *sphere = load_model(iqe_sphere, iqe_sphere, &t_tiles);
	sphere->flags = 0;
	model *knight = load_model(iqe_knight, iqe_knight, &t_tiles);
	knight->flags = MODEL_FLAG_DRAW;
	unload_model_raw(iqe_sphere);
	unload_model_raw(iqe_knight);

/*
	model_raw iqe_lvl = loadiqe("res/lvl/level1_vc.iqe");
	model_raw iqe_lvl_col = loadiqe("res/lvl/level1_vc_col.iqe");
	model *m = load_model(iqe_lvl, iqe_lvl_col, &t_floor);
	m->flags = MODEL_FLAG_COLLIDE | MODEL_FLAG_DRAW;
	unload_model_raw(iqe_lvl);
	unload_model_raw(iqe_lvl_col);

*/
	model_raw iqe_wall = loadiqe("res/lvl/level1_wall.iqe");
	model_raw iqe_floor = loadiqe("res/lvl/level1_floor.iqe");
	
	model_raw iqe_wall_col = loadiqe("res/lvl/level1_wall_col.iqe");
	model_raw iqe_floor_col = loadiqe("res/lvl/level1_floor_col.iqe");

	model *m = load_model(iqe_floor, iqe_floor, &t_floor);
	m->flags = MODEL_FLAG_COLLIDE | MODEL_FLAG_DRAW;
	model *m2 = load_model(iqe_wall, iqe_wall, &t_wall);
	m2->flags = MODEL_FLAG_COLLIDE | MODEL_FLAG_DRAW;

	unload_model_raw(iqe_wall);
	unload_model_raw(iqe_floor);
	unload_model_raw(iqe_wall_col);
	unload_model_raw(iqe_floor_col);


	player p = player_init((vec3f){0.f, 0.f, 30.f});
//	mob *b = mob_add((vec3f){35.f, -10.f, 0.f}, (vec2f){1.f, 1.f}, (vec3f){1.f, 1.f, 1.f}, knight);

	camera cam = {0};
	CAMERA = &cam;
	CAMERA->pos = (vec3f){-7.0f,15.0f,9.0f};
	camera_target_from_angle(CAMERA);
	CAMERA->up = (vec3f){0.0f,1.0f,0.0f};

	CAMERA->proj = mat_project(90.f*(PI/180.f), 9.f/16.f, CLIP_NEAR, 1000.f);
	CAMERA->view = mat_lookat(CAMERA->pos, CAMERA->target, CAMERA->up);

	int rtime = 0;
	int gtime = 0;

	while(!input_key(KEYMAP_QUIT))
	{
// game start
		game_frametime_update();

		input_flush();
		game_run(&p, sphere);
		game_flush();

		game_frametime_update();
// game end
// render start
		render_frametime_update();

		CAMERA->pos = p.pos;
		CAMERA->face = p.face;
		camera_target_from_angle(CAMERA);
		CAMERA->up = (vec3f){0.f, 0.f, 1.f};
		CAMERA->view = mat_lookat(CAMERA->pos, CAMERA->target, CAMERA->up);

		render_run();
//		render_flush();

// debug start
		sprintf(debug_text, "TIME: %i %i\npos: %f %f %f\ntar: %f %f %f\nflags: %i\nzbuf: %i\n",
							gtime, rtime,
							p.pos.x, p.pos.y, p.pos.z, 
							CAMERA->target.x, CAMERA->target.y, CAMERA->target.z, 
							p.flags,
							//RENDER_DATA.zbuf[(RENDER_DATA.width/2) + (RENDER_DATA.height/2)*RENDER_DATA.width]
							DEPTH
							);
		render_px(960, 540, color_rgb(255,0,255));
		char models_text[1024];
		print_mem(GAME_MEM, 0, models_text);
		strcat(debug_text, models_text);
		print_mem(RENDER_MEM, 0, models_text);
		strcat(debug_text, models_text);
		print_models(models_text);
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
