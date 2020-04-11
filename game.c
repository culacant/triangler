#include "game.h"
player player_init(vec3f p)
{
	player out;
	out.pos = p;
	out.vel = (vec3f){0.0f, 0.0f, 0.0f};
	out.face = 0.0f;
	out.r = (vec3f){1/1.0f, 1/1.0f, 1/2.0f};
	return out;
}
void player_free()
{
}

void player_vel_from_face(player *p)
{
	float sa = sinf(p->face);
	float ca = cosf(p->face);
	vec3f ovel = p->vel;

	p->vel.x = sa*ovel.x + ca*ovel.y;
	p->vel.y = ca*ovel.x - sa*ovel.y;
}
void player_collide(player *p, model m)
{
	int try = 0;
	int collided = 0;
	collision col = {0};
	col.pos = vec_mul(p->pos, p->r);
	col.vel = vec_mul(p->vel, p->r);;
	while(try < 3)
	{
		for(int i=0;i<m.fcnt;i++)
		{
			vec3f a = vec_mul(m.vp[m.fm[i*3+0]], p->r);
			vec3f b = vec_mul(m.vp[m.fm[i*3+1]], p->r);
			vec3f c = vec_mul(m.vp[m.fm[i*3+2]], p->r);
			vec3f n = m.fn[i];
			collided += swept_tri_collision(col.pos, col.vel, a, b, c, n, &col);
		}
		if(collided == 0)
			break;
		try++;
	}
	p->pos = vec_div(col.pos, p->r);
	p->vel = (vec3f){0.0f, 0.0f, 0.0f};
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
		cur = &PROJECTILES.arr[PROJECTILES.it];
		if(cur->ttl > 0)
		{	
			cur->ttl -= dt;
			cur->pos = vec_add(cur->pos, cur->vel);
			cur->m.trans = mat_transform(cur->pos);
			drawmodel_tex(cur->m, *cur->t);
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
