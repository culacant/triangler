#include "game.h"

void game_init()
{
	GAME_DATA.modelcnt = 0;
	GAME_DATA.tricnt = 0;
	game_frametime_update();
}
void game_flush()
{
	RENDER_DATA.modelcnt = GAME_DATA.modelcnt;
	memcpy(RENDER_DATA.models, GAME_DATA.models, sizeof(model)*GAME_DATA.modelcnt);
	game_frametime_update();
}

void game_run(player *p , model *m, model *sphere)
{       
	if(INPUT_DATA.mouseactivity)
	{
		p->face.x -= (float)(input_mouse_relx()*MOUSE_SENSITIVITY);
		p->face.y += (float)(input_mouse_rely()*MOUSE_SENSITIVITY);
	}
	if(input_key(KEY_W))
		p->impulse.x += 0.002f;
	if(input_key(KEY_S))
		p->impulse.x -= 0.002f;
	if(input_key(KEY_A))
		p->impulse.y += 0.002f;
	if(input_key(KEY_D))
		p->impulse.y -= 0.002f;
	if(input_key(KEY_SPACE))
		p->impulse.z += JUMP_HEIGHT;

	player_update_vel(p);
	player_update_muzzle(p);
	player_collide(p);
	p->vel = GRAVITY;
	player_collide(p);
	p->pos.z += GRAVITY.z;

	if(input_key(KEY_R))
	{
		projectile bullet;
		bullet.ttl = 100;
		bullet.pos = p->muzzle;
		float sx = sin(p->face.x)*10;
		float cx = cos(p->face.x)*10;
		float sy = sin(p->face.y)*10;
		bullet.vel = (vec3f){sx,cx,sy};
		bullet.m = dupe_model(sphere);
		bullet.m->flags = FLAG_DRAW;
		bullet.radius = 1/0.1f;
		projectile_add(bullet);
	}

	projectiles_tick(1, m);

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
	out.r = (vec3f){1/1.0f, 1/1.0f, 1/2.0f};
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

	if(p->flags & FLAG_AIR)
		p->impulse = vec3f_scale(p->impulse, JUMP_FRAC);
	if(p->flags & FLAG_GND)
	{
		p->vel = vec3f_scale(p->vel, DRAG_FRAC);
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
			if(cur->flags & FLAG_COLLIDE)
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
					col.pos = vec3f_add(col.pos, vec3f_scale(n, SMALLNR));

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
//	p->vel = (vec3f){0.0f, 0.0f, 0.0f};
	p->impulse = (vec3f){0.0f, 0.0f, 0.0f};
}

void projectiles_init()
{
	PROJECTILES.it = 0;
	for(int i=0;i<PROJECTILECNT;i++)
		PROJECTILES.arr[i].ttl = -1;
}
void projectiles_free()
{
	PROJECTILES.it = 0;
}
int projectile_add(projectile p)
{
	int cnt = 0;
	while(cnt < PROJECTILECNT)
	{
		if(PROJECTILES.arr[PROJECTILES.it].ttl <= 0)
		{
			PROJECTILES.arr[PROJECTILES.it] = p;
			return 1;
		}
		PROJECTILES.it = (PROJECTILES.it+1) & PROJECTILECNT;
		cnt++;
	}
	return 0;
}

void projectiles_tick(int dt, model *m)
{
	projectile *cur;
	for(int i=0;i<PROJECTILECNT;i++)
	{
		cur = &PROJECTILES.arr[i];
		if(cur->ttl >= 0)
		{	
			cur->ttl -= dt;
			cur->pos = vec3f_add(cur->pos, cur->vel);
			cur->m->trans = mat_transform(cur->pos);
			if(projectile_collide(cur, m) || cur->ttl < 0)
			{
				cur->ttl = -1;
				cur->m->flags &= ~FLAG_DRAW;
			}
		}	
	}
}
int projectile_collide(projectile *p, model *m)
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

void mobs_init()
{
	MOBS.it = 0;
	for(int i=0;i<MOBCNT;i++)
		MOBS.arr[i].flags = 0;
}
void mobs_free()
{
	MOBS.it = 0;
}
int mob_add(mob m)
{
	int cnt = 0;
	while(cnt < MOBCNT)
	{
		if(MOBS.arr[MOBS.it].flags == 0)
		{
			MOBS.arr[MOBS.it] = m;
			return 1;
		}
		MOBS.it = (MOBS.it+1) & MOBCNT;
		cnt++;
	}
	return 0;
}
