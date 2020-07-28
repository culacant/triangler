#include "game.h"

void game_init()
{
	GAME_MEM = mem_init(GAME_MEM_SIZE);

	GAME_DATA.modelcnt = 0;
	GAME_DATA.modelit = 0;
	GAME_DATA.models = mem_alloc(sizeof(model)*MODEL_CNT, GAME_MEM);
	for(int i=0;i<MODEL_CNT;i++)
	{
		GAME_DATA.models[i].flags = MODEL_FLAG_FREE;
	}

	GAME_DATA.tricnt = 0;
	GAME_DATA.tris= mem_alloc(sizeof(game_triangle)*TRIANGLE_CNT, GAME_MEM);

	GAME_DATA.mobcnt = 0;
	GAME_DATA.mobit = 0;
	GAME_DATA.mobs = mem_alloc(sizeof(mob)*MOB_CNT, GAME_MEM);
	for(int i=0;i<MOB_CNT;i++)
	{
		GAME_DATA.mobs[i].flags = MOB_FLAG_FREE;
	}

	GAME_DATA.bulletcnt = 0;
	GAME_DATA.bulletit = 0;
	GAME_DATA.bullets= mem_alloc(sizeof(bullet)*BULLET_CNT, GAME_MEM);
	for(int i=0;i<BULLET_CNT;i++)
	{
		GAME_DATA.bullets[i].ttl = -1;
		GAME_DATA.bullets[i].m = NULL;
	}
}
void game_free()
{
	mem_free(GAME_DATA.models);
	mem_free(GAME_DATA.tris);
	mem_free(GAME_DATA.mobs);
	mem_free(GAME_DATA.bullets);
	free(GAME_MEM);
}
void game_flush()
{
	RENDER_DATA.modelcnt = GAME_DATA.modelcnt;
	memcpy(RENDER_DATA.models, GAME_DATA.models, sizeof(model)*MODEL_CNT);
}

void game_run(player *p , model *m, model *sphere)
{       
	if(INPUT_DATA.mouseactivity)
	{
		p->face.x -= (float)(input_mouse_relx()*MOUSE_SENSITIVITY);
		p->face.y += (float)(input_mouse_rely()*MOUSE_SENSITIVITY);
	}
	if(input_key(KEY_W))
		p->impulse.x += IMPULSE;
	if(input_key(KEY_S))
		p->impulse.x -= IMPULSE;
	if(input_key(KEY_A))
		p->impulse.y += IMPULSE;
	if(input_key(KEY_D))
		p->impulse.y -= IMPULSE;
	if(input_key(KEY_SPACE))
		p->impulse.z += JUMP_HEIGHT;

	p->impulse.z += GRAVITY.z;
	player_update_vel(p);
	player_collide(p);

	player_update_muzzle(p);

	if(input_key(KEY_R))
	{
		bullet bullet;
		bullet.pos = p->muzzle;
		float sx = sin(p->face.x)*10;
		float cx = cos(p->face.x)*10;
		float sy = sin(p->face.y)*10;
		bullet.vel = (vec3f){sx,cx,sy};
		bullet.radius = 1/0.1f;
		bullet_add(bullet.pos, bullet.vel, bullet.radius, sphere);
	}

	bullets_tick(1);
	mobs_tick(1);

}

void game_frametime_update()
{
    static int lasttime;
    int curtime;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    curtime = (int)(ts.tv_sec*1000 + ts.tv_nsec/1000000);
    GAME_DATA.frametime = curtime - lasttime;
    lasttime = curtime;
}

void input_init()
{
    INPUT_DATA.fd_kb = open(KB_NAME, O_RDONLY);
    INPUT_DATA.fd_mouse = open(MOUSE_NAME, O_RDONLY);
    fcntl(INPUT_DATA.fd_mouse, F_SETFL, O_NONBLOCK);

    if(INPUT_DATA.fd_kb <= 0)
    {    
        printf("ERROR: cannot open input: %s\n", KB_NAME);
        return;
    }    

    memset(INPUT_DATA.keys, 0, sizeof(INPUT_DATA.keys));

    if(INPUT_DATA.fd_mouse<= 0)
    {    
        printf("ERROR: cannot open input: %s\n", MOUSE_NAME);
        return;
    }    
    memset(INPUT_DATA.mousedata, 0, sizeof(INPUT_DATA.mousedata));
    INPUT_DATA.mouseactivity = 0; 
    INPUT_DATA.mouseshow = 0; 
    INPUT_DATA.mousex = 0; 
    INPUT_DATA.mousey = 0; 

    input_flush();
}
void input_free()
{
    close(INPUT_DATA.fd_kb);
    close(INPUT_DATA.fd_mouse);
}
void input_flush()
{
    memset(INPUT_DATA.mousedata, 0, sizeof(INPUT_DATA.mousedata));
    ioctl(INPUT_DATA.fd_kb, EVIOCGKEY(sizeof(INPUT_DATA.keys)), INPUT_DATA.keys);

    INPUT_DATA.mouseactivity = read(INPUT_DATA.fd_mouse, INPUT_DATA.mousedata, sizeof(INPUT_DATA.mousedata));

    if(INPUT_DATA.mouseshow)
    {    
        INPUT_DATA.mousex += input_mouse_relx();
        INPUT_DATA.mousey += input_mouse_rely();
    }    
}
int input_key(int key) 
{
    return !!(INPUT_DATA.keys[key/8] & (1<<(key%8)));
}
int input_mouse_button(char button)
{
    return INPUT_DATA.mousedata[0] & button;
}
int input_mouse_relx()
{
    return (int)INPUT_DATA.mousedata[1];
}
int input_mouse_rely()
{
    return (int)INPUT_DATA.mousedata[2];
}
int input_mouse_absx()
{
    return INPUT_DATA.mousex;
}
int input_mouse_absy()
{
    return INPUT_DATA.mousey;
}

player player_init(vec3f p)
{
	player out;
	out.pos = p;
	out.vel = (vec3f){0.0f, 0.0f, 0.0f};
	out.impulse = (vec3f){0.0f, 0.0f, 0.0f};
	out.face = (vec2f){0.0f, 0.0f};
	out.r = (vec3f){1/1.4f, 1/1.4f, 1/2.0f};
	out.flags = FLAG_NONE;

	out.muzzle_ofs = (vec3f){1.0f, 0.0f, 1.0f};
	return out;
}
void player_free()
{
}

void player_update_vel(player *p)
{
	float sa = sinf(p->face.x);
	float ca = cosf(p->face.x);
/*
	if(p->flags & FLAG_AIR)
		p->impulse = vec3f_scale(p->impulse, JUMP_FRAC);
*/
	p->vel = vec3f_scale(p->vel, DRAG_FRAC);
	if(p->flags & FLAG_GND)
	{
		p->vel.x += sa*p->impulse.x + ca*p->impulse.y;
		p->vel.y += ca*p->impulse.x - sa*p->impulse.y;
	}
	p->vel.z += p->impulse.z;
}
void player_update_muzzle(player *p)
{
	float sx = sinf(p->face.x);
	float cx = cosf(p->face.x);
	float sy = sinf(p->face.y);

	p->muzzle.x = p->pos.x + (sx*p->muzzle_ofs.x + cx*p->muzzle_ofs.y);
	p->muzzle.y = p->pos.y + (cx*p->muzzle_ofs.x - sx*p->muzzle_ofs.y);
	p->muzzle.z = p->pos.z + sy*p->muzzle_ofs.z;
}
void player_collide(player *p)
{
	int try = 0;
	int collided = 0;
	float n_maxz = -2.0f;
	collision col = {0};
	col.pos = vec3f_mul(p->pos, p->r);
	col.vel = vec3f_mul(p->vel, p->r);;
	model *cur;

	while(try < 3)
	{
		for(int m=0;m<GAME_DATA.modelcnt;m++)
		{
			cur = &GAME_DATA.models[m];
			if(cur->flags & MODEL_FLAG_COLLIDE)
			{
				for(int i=0;i<cur->gtricnt;i++)
				{
		// trans only for moving objects
					vec3f a = vec3f_mul(cur->gtri[i].a, p->r);
					vec3f b = vec3f_mul(cur->gtri[i].b, p->r);
					vec3f c = vec3f_mul(cur->gtri[i].c, p->r);
		// FIXME: transform n
					vec3f n = cur->gtri[i].n;
					int res = swept_tri_collision(col.pos, col.vel, a, b, c, n, &col);
// causes twitchy gravity
//					col.pos = vec3f_add(col.pos, vec3f_scale(n, SMALLNR));

					if((res > 0) && (n.z > n_maxz))
						n_maxz = n.z;
					collided += res;
				}
			}
		}
		if(collided == 0)
		{
			p->flags = FLAG_AIR;
			break;
		}
		else if(n_maxz < FLOOR_Z_TRESHOLD)
			p->flags = FLAG_AIR;
		else
			p->flags = FLAG_GND;
		try++;
	}
	p->pos = vec3f_div(col.pos, p->r);
	p->vel = (vec3f){0.0f, 0.0f, 0.0f};
	p->impulse = (vec3f){0.0f, 0.0f, 0.0f};
}

mob *mob_add(vec3f pos, vec2f face, vec3f radius, model *m)
{	
	model *mod = dupe_model(m);
	if(!mod)
		return NULL;


	mob *out = malloc_game_tri(1);
	if(!out)
	{
		free_model(mod);
		return NULL;
	}
	
	out->pos = pos;
	out->face = face;
	out->radius = radius;

	out->m = mod;
	out->m->flags = MODEL_FLAG_DRAW;
	out->m->trans= mat_transform(out->pos);;

	return out;
}
void mobs_tick(int dt)
{
	int cnt = 0;
	mob *cur;
	for(int i=0;i<MOB_CNT;i++)
	{
		cur = &GAME_DATA.mobs[i];
		if(!(cur->flags & MOB_FLAG_FREE))
		{
			cur->m->trans = mat_transform(cur->pos);

			cnt++;
			if(cnt >= GAME_DATA.mobcnt)
				break;
		}
	}
}

bullet* bullet_add(vec3f pos, vec3f vel, float radius, model *m)
{
	model *mod = dupe_model(m);
	if(!mod)
		return NULL;

	bullet *out = malloc_game_bullet(1);
	if(!out)
	{
		free_model(mod);
		return NULL;
	}
	
	out->ttl = 20;
	out->pos = pos;
	out->vel = vel;
	out->radius = radius;

	out->m = mod;
	out->m->flags = MODEL_FLAG_DRAW;
	out->m->trans= mat_transform(out->pos);;

	return out;
}
void bullets_tick(int dt)
{
	int cnt = 0;
	bullet *cur;
	for(int i=0;i<BULLET_CNT;i++)
	{
		cur = &GAME_DATA.bullets[i];
		if(cur->ttl >= 0)
		{
			cur->ttl -= dt;
			cur->pos = vec3f_add(cur->pos, cur->vel);
			cur->m->trans = mat_transform(cur->pos);
			if(cur->ttl < 0)
				free_game_bullet(cur);
			else
			{
				for(int m=0;m<MODEL_CNT;m++)
				{
					if(GAME_DATA.models[m].flags & MODEL_FLAG_COLLIDE)
						if(bullet_collide(cur, &GAME_DATA.models[m]))
							free_game_bullet(cur);
				}
			}
			cnt++;
			if(cnt >= GAME_DATA.bulletcnt)
				break;
		}
	}
}
int bullet_collide(bullet *p, model *m)
{
	collision col = {0};
	vec3f pos = vec3f_scale(p->pos, p->radius);
	vec3f vel = vec3f_scale(p->vel, p->radius);

	for(int i=0;i<m->gtricnt;i++)
	{
// trans only for moving objects
		vec3f a = vec3f_scale(m->gtri[i].a, p->radius);;
		vec3f b = vec3f_scale(m->gtri[i].b, p->radius);;
		vec3f c = vec3f_scale(m->gtri[i].c, p->radius);;
		vec3f n = m->gtri[i].n;

		if(swept_tri_collision(pos, vel, a, b, c, n, &col) != COLLISION_FALSE)
			return COLLISION_TRUE;
	}
	return COLLISION_FALSE;
}
