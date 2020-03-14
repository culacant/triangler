#include "render.h"

#define EPSILON 0.000001f

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
	float dist_sign;
	float t0;
	float t1;
	float t_div = vec_dot(n,vel);
	if(t_div != 0.0f)
	{
		dist_sign = vec_dot(n, vec_sub(pos, a));
		t0 = (1-dist_sign)/t_div;
		t1 = (-1-dist_sign)/t_div;
	}
	int i;
	int i2;
// check same sign
	if((i*i2) > 0)
		return 1;
	return 0;
}
