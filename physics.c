#include "render.h"

#define EPSILON 0.000001f

int ray_tri_collision(vec3f o, vec3f dir, vec3f a, vec3f b, vec3f c, vec3f *out)
{
	vec3f e1;
	vec3f e2;
	vec3f t;
	vec3f p;
	vec3f q;
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
		out->x = vec_dot(t, p);
		if(out->x < 0.0f || out->x > det)
			return 0;

		out->y = vec_dot(dir, q);
		if(out->y < 0.0f || (out->x+out->y) > det)
			return 0;
	}
	else if( det < -EPSILON)
	{
		out->x = vec_dot(t, p);
		if(out->x > 0.0f || out->x < det)
			return 0;

		out->y = vec_dot(dir, q);
		if(out->y > 0.0f || (out->x+out->y) < det)
			return 0;
	}
	else
		return 0;

	out->z = vec_dot(e2, q) * det_inv;
	out->x *= det_inv;
	out->y *= det_inv;

	return 1;
}
int swept_tri_collision(vec3f sphere, vec3f vel, vec3f a, vec3f b, vec3f c, vec3f n)
{
	int i;
	int i2;
// check same sign
	if((i*i2) > 0)
		return 1;
	return 0;
}
