#include "render.h"

#define EPSILON 0.000001f

int point_in_tri(vec3f p, vec3f a, vec3f b, vec3f c)
{
	vec3f e10 = vec_sub(b,a);
	vec3f e20 = vec_sub(c,a);

	float u = vec_dot(e10,e10);
	float v = vec_dot(e10,e20);
	float w = vec_dot(e20,e20);
	float uw_vv = (u*w)-(v*v);
	vec3f vp = (vec3f){p.x-a.x, p.y-a.y, p.z-a.z};

	float d = vec_dot(vp, e10);
	float e = vec_dot(vp, e20);
	float x = (d*w) - (e*v);
	float y = (e*u) - (d*v);
	float z = x+y-uw_vv;

	unsigned int ix;
	unsigned int iy;
	unsigned int iz;
	memcpy(&ix, &x, sizeof(float));	// unsigned int ix = (unsigned int&)x;
	memcpy(&iy, &y, sizeof(float));
	memcpy(&iz, &z, sizeof(float));

	return ((iz & ~(ix|iy)) & 0x80000000);		// real shit
}

int ray_tri_collision(vec3f o, vec3f dir, vec3f a, vec3f b, vec3f c, intersection *out)
{
	vec3f e1;
	vec3f e2;
	vec3f t;
	vec3f p;
	vec3f q;
	vec3f u;
	float det;
	float det_inv;

	e1 = vec_sub(b, a);
	e2 = vec_sub(c, a);

	p = vec_cross(dir, e2);
	det = vec_dot(e1, p);

	t = vec_sub(o, a);
	det_inv = 1.0f/det;

	q = vec_cross(t, e1);

	if(det > EPSILON)
	{
		u.x = vec_dot(t, p);
		if(u.x < 0.0f || u.x > det)
			return 0;

		u.y = vec_dot(dir, q);
		if(u.y < 0.0f || (u.x+u.y) > det)
			return 0;
	}
	else if( det < -EPSILON)
	{
		u.x = vec_dot(t, p);
		if(u.x > 0.0f || u.x < det)
			return 0;

		u.y = vec_dot(dir, q);
		if(u.y > 0.0f || (u.x+u.y) < det)
			return 0;
	}
	else
		return 0;

	out->distance = vec_dot(e2, q) * det_inv;
	out->pos = vec_add(o, vec_mul_f(dir, out->distance));
	out->normal = vec_norm(vec_cross(e1, e2));

	return 1;
}
int swept_tri_collision(vec3f pos, vec3f vel, vec3f a, vec3f b, vec3f c, vec3f n, intersection *out)
{
	return 0;
}
/*
{
	float t;
	float t0;
	float t1;
	float tmp;
	float t_div = vec_dot(n,vel);
	float dist_sign = vec_dot(n, vec_sub(pos, a));
	int embedded = 0;
	int collision = 0;

	vec3f plane_intersect;

	if(t_div == 0.0f)
	{
		if(fabs(dist_sign) >= 1.0f)		// unit sphere
			return 0;
		else
		{
			embedded = 1;
			t0 = 0.0f;
			t1 = 1.0f;
		}
	}
	else
	{
		t0 = (1.0f-dist_sign)/t_div;
		t1 = (-1.0f-dist_sign)/t_div;
		if(t0 > t1)
		{
			tmp = t0;
			t0 = t1;
			t1 = tmp;
		}
			
		if(t0 > 1.0f || t1 < 0.0f)
			return 0;

		if(t0 < 0.0f)
			t0 = 0.0f;
		if(t0 > 1.0f)
			t0 = 1.0f;
		if(t1 < 0.0f)
			t1 = 0.0f;
		if(t1 > 1.0f)
			t1 = 1.0f;
	}
	if(!embedded)
	{
		plane_intersect = vec_sub(pos, vec_add(n, vec_mul_f(vel, t0)));
		if(point_in_tri(plane_intersect, a, b, c))
		{
			collision = 1;
			t = t0;
			out->pos = plane_intersect;
		}
	}

	if(!collision)
	{
		
	}
	return 1;
}
*/
