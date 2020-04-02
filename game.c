#include "game.h"
player player_init(vec3f p)
{
	player out;
	out.pos = p;
	out.vel = (vec3f){0.0f, 0.0f, 0.0f};
	out.face = 0.0f;
	out.r = 1.5f;
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
	col.pos = p->pos;
	col.vel = p->vel;
	while(try < 5)
	{
		for(int i=0;i<m.fcnt;i++)
		{
			vec3f a = m.vp[m.fm[i*3+0]];
			vec3f b = m.vp[m.fm[i*3+1]];
			vec3f c = m.vp[m.fm[i*3+2]];
			vec3f n = m.fn[i];
			collided += swept_tri_collision(col.pos, p->r, col.vel, a, b, c, n, &col);
		}
		try++;
		if(collided == 0)
			break;
	}
	p->pos = col.pos;
	p->vel = (vec3f){0.0f, 0.0f, 0.0f};
}
