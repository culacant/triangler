#include "render.h"

int point_in_tri(vec3f p, vec3f a, vec3f b, vec3f c)
{
	vec3f e10 = vec3f_sub(b,a);
	vec3f e20 = vec3f_sub(c,a);

	float u = vec3f_dot(e10,e10);
	float v = vec3f_dot(e10,e20);
	float w = vec3f_dot(e20,e20);
	float uw_vv = (u*w)-(v*v);
	vec3f vp = vec3f_sub(p, a);

	float d = vec3f_dot(vp, e10);
	float e = vec3f_dot(vp, e20);
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

int ray_tri_intersect(vec3f o, vec3f dir, vec3f a, vec3f b, vec3f c, intersection *out)
{
	vec3f e1;
	vec3f e2;
	vec3f t;
	vec3f p;
	vec3f q;
	vec3f u;
	float det;
	float det_inv;

	e1 = vec3f_sub(b, a);
	e2 = vec3f_sub(c, a);

	p = vec3f_cross(dir, e2);
	det = vec3f_dot(e1, p);

	t = vec3f_sub(o, a);
	det_inv = 1.0f/det;

	q = vec3f_cross(t, e1);

	if(det > SMALLNR)
	{
		u.x = vec3f_dot(t, p);
		if(u.x < 0.0f || u.x > det)
			return 0;

		u.y = vec3f_dot(dir, q);
		if(u.y < 0.0f || (u.x+u.y) > det)
			return 0;
	}
	else if( det < -SMALLNR)
	{
		u.x = vec3f_dot(t, p);
		if(u.x > 0.0f || u.x < det)
			return 0;

		u.y = vec3f_dot(dir, q);
		if(u.y > 0.0f || (u.x+u.y) < det)
			return 0;
	}
	else
		return 0;

	out->distance = vec3f_dot(e2, q) * det_inv;
	out->pos = vec3f_add(o, vec3f_scale(dir, out->distance));
	out->normal = vec3f_norm(vec3f_cross(e1, e2));

	return 1;
}
int swept_tri_collision(vec3f pos, vec3f vel, vec3f a, vec3f b, vec3f c, vec3f n, collision *out)
{
	vec3f pv = vec3f_add(pos, vel);
	float l;

	vec3f v1;
	float d1;
	vec3f a1 = vec3f_sub(a, pos);
	vec3f b1 = vec3f_sub(b, pos);
	vec3f c1 = vec3f_sub(c, pos);

	vec3f v2;
	float d2;
	float e2;
	vec3f a2 = vec3f_sub(a, pv);
	vec3f b2 = vec3f_sub(b, pv);
	vec3f c2 = vec3f_sub(c, pv);

	v1 = vec3f_cross(vec3f_sub(b1, a1), vec3f_sub(c1, a1));
	d1 = vec3f_dot(a1, v1);

	v2 = vec3f_cross(vec3f_sub(b2, a2), vec3f_sub(c2, a2));
	d2 = vec3f_dot(a2, v2);
	e2 = vec3f_dot(v2, v2);

	if((d1 * d2) <= 0)
	{
		l = inv_lerp(d1, d2, 0);
		out->pos = vec3f_lerp(pos, pv, l);
		if(point_in_tri(out->pos, a, b, c))
		{
			out->pos = vec3f_add(out->pos, n);
			out->vel = vec3f_sub(vec_project_plane(pv, out->pos, n), out->pos);
			out->distance2 = vec3f_len2(vel);
			return COLLISION_TRUE;
		}
		else
        {
            float pab = vec3f_dist2(vec_project_segment(out->pos, a, b), out->pos);
            float pac = vec3f_dist2(vec_project_segment(out->pos, a, c), out->pos);
            float pbc = vec3f_dist2(vec_project_segment(out->pos, b, c), out->pos);
            if(pab < 1.0f || pac < 1.0f || pbc < 1.0f)
            {
                out->pos = vec3f_add(out->pos, n);
                out->vel = vec3f_sub(vec_project_plane(pv, out->pos, n), out->pos);
                out->distance2 = vec3f_len2(vel);
                return COLLISION_TRUE;
            }
        }
	}
	else if((d2*d2) <= e2)
	{
		if(point_in_tri(pv, a, b, c))
		{
			out->pos = vec_project_plane(pv, a, n);

			out->pos = vec3f_add(out->pos, n);
			out->vel = (vec3f) {0.0f, 0.0f, 0.0f};
			out->distance2 = 0.0f;
			return COLLISION_DONE;
		}
		else
        {
            float pab = vec3f_dist2(vec_project_segment(pv, a, b), out->pos);
            float pac = vec3f_dist2(vec_project_segment(pv, a, c), out->pos);
            float pbc = vec3f_dist2(vec_project_segment(pv, b, c), out->pos);
            if(pab < 1.0f || pac < 1.0f || pbc < 1.0f)
            {
                out->pos = vec_project_plane(pv, a, n);
                out->pos = vec3f_add(out->pos, n);
                out->vel = (vec3f) {0.0f, 0.0f, 0.0f};
                out->distance2 = 0.0f;
                return COLLISION_DONE;
            }
        }
	}
	// dont need to check d1
	out->pos = pv;
	out->vel = vel;
	out->distance2 = vec3f_len2(vel);
	return COLLISION_FALSE;
}
int swept_sphere_collision(vec3f pos1, vec3f vel1, float radius1, vec3f pos2, float radius2, collision *out)
{
	return 0;
}

