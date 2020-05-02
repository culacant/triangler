#include "game.h"

void game_init()
{
	GAME_DATA.modelcnt = 0;
	GAME_DATA.tricnt = 0;
}
void game_flush()
{
	RENDER_DATA.modelcnt = GAME_DATA.modelcnt;
	memcpy(RENDER_DATA.models, GAME_DATA.models, sizeof(model)*GAME_DATA.modelcnt);
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
	float sa = sinf(p->face.x);
	float ca = cosf(p->face.x);

	p->muzzle.x = p->pos.x + (sa*p->muzzle_ofs.x + ca*p->muzzle_ofs.y);
	p->muzzle.y = p->pos.y + (ca*p->muzzle_ofs.x - sa*p->muzzle_ofs.y);
	p->muzzle.z = p->pos.z + p->muzzle_ofs.z;
}
void player_collide(player *p, model *m)
{
	int try = 0;
	int collided = 0;
	float n_maxz = -2.0f;
	collision col = {0};
	col.pos = vec3f_mul(p->pos, p->r);
	col.vel = vec3f_mul(p->vel, p->r);;

	while(try < 3)
	{
		for(int i=0;i<m->gtricnt;i++)
		{
// trans only for moving objects
			vec3f a = vec3f_trans(vec3f_mul(m->gtri[i].a, p->r),m->trans);
			vec3f b = vec3f_trans(vec3f_mul(m->gtri[i].b, p->r),m->trans);
			vec3f c = vec3f_trans(vec3f_mul(m->gtri[i].c, p->r),m->trans);
// FIXME: transform n
			vec3f n = m->gtri[i].n;
			int res = swept_tri_collision(col.pos, col.vel, a, b, c, n, &col);
			col.pos = vec3f_add(col.pos, vec3f_scale(n, SMALLNR));

			if((res > 0) && (n.z > n_maxz))
				n_maxz = n.z;
			collided += res;
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
	p->pos.z += SMALLNR;
//	p->vel = (vec3f){0.0f, 0.0f, 0.0f};
	p->impulse = (vec3f){0.0f, 0.0f, 0.0f};
}

void projectiles_init()
{
	PROJECTILES.it = 0;
}
void projectiles_free()
{
	PROJECTILES.it = 0;
}
void projectiles_tick(int dt)
{
	projectile *cur;
	for(int i=0;i<PROJECTILECNT;i++)
	{
		cur = &PROJECTILES.arr[i];
		if(cur->ttl > 0)
		{	
			cur->ttl -= dt;
			cur->pos = vec3f_add(cur->pos, cur->vel);
			cur->m->trans = mat_transform(cur->pos);
//			drawmodel_tex(cur->m, *cur->t);
		}	
	}
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
