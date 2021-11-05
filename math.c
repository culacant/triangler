#include "render.h"

vec3f vec3f_cross(vec3f a, vec3f b)
{
	vec3f out = {a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x};
	return out;
}
float vec3f_dot(vec3f a, vec3f b)
{
	return a.x*b.x+a.y*b.y+a.z*b.z;
}
float vec3f_len(vec3f a)
{
	return sqrtf(a.x*a.x + a.y*a.y + a.z*a.z);
}
float vec3f_len2(vec3f a)
{
	return (a.x*a.x + a.y*a.y + a.z*a.z);
}
int vec3i_dot(vec3i a, vec3i b)
{
	return a.x*b.x+a.y*b.y+a.z*b.z;
}
float vec3f_dist(vec3f a, vec3f b)
{
	float dx = b.x-a.x;
	float dy = b.y-a.y;
	float dz = b.z-a.z;
	return sqrtf(dx*dx + dy*dy + dz*dz);
}
float vec3f_dist2(vec3f a, vec3f b)
{
	float dx = b.x-a.x;
	float dy = b.y-a.y;
	float dz = b.z-a.z;
	return (dx*dx + dy*dy + dz*dz);
}
vec3f vec3f_add(vec3f a, vec3f b)
{
	vec3f out;
	out.x = a.x+b.x;
	out.y = a.y+b.y;
	out.z = a.z+b.z;
	return out;
}
vec3i vec3i_add(vec3i a, vec3i b)
{
	vec3i out;
	out.x = a.x+b.x;
	out.y = a.y+b.y;
	out.z = a.z+b.z;
	return out;
}
vec3f vec3f_sub(vec3f a, vec3f b)
{
	vec3f out;
	out.x = a.x-b.x;
	out.y = a.y-b.y;
	out.z = a.z-b.z;
	return out;
}
vec3i vec3i_sub(vec3i a, vec3i b)
{
	vec3i out;
	out.x = a.x-b.x;
	out.y = a.y-b.y;
	out.z = a.z-b.z;
	return out;
}
vec3f vec3f_mul(vec3f a, vec3f b)
{
	return (vec3f){a.x*b.x, a.y*b.y, a.z*b.z};
}
vec3f vec3f_scale(vec3f a, float f)
{
	return (vec3f){a.x*f, a.y*f, a.z*f};
}
vec3f vec3f_div(vec3f a, vec3f b)
{
	return (vec3f){a.x/b.x, a.y/b.y, a.z/b.z};
}
vec3f vec3f_norm(vec3f a)
{
	float l = sqrtf(a.x*a.x+a.y*a.y+a.z*a.z);
	float il;
	if(l == 0.0f)
		l = 1.0f;
	il = 1.0f/l;
	a.x *= il;
	a.y *= il;
	a.z *= il;
	return a;
}
vec3f barycentric(vec3f a, vec3f b, vec3f c, vec3f p)
{
	vec3f out = {0};
	vec3f v0 = vec3f_sub(b,a);
	vec3f v1 = vec3f_sub(c,a);
	vec3f v2 = vec3f_sub(p,a);
	float d00 = vec3f_dot(v0,v0);
	float d01 = vec3f_dot(v0,v1);
	float d11 = vec3f_dot(v1,v1);
	float d20 = vec3f_dot(v2,v0);
	float d21 = vec3f_dot(v2,v1);
	float denominv = 1.0f/(d00*d11 - d01*d01);
	out.x = (d11*d20 - d01*d21)*denominv;
	out.y = (d00*d21 - d01*d20)*denominv;
	out.z = 1.0f-out.x-out.y;
	return out;
}
vec3f barycentric_i(vec3i a, vec3i b, vec3i c, vec3i p)
{
	vec3f out = {0};
	vec3i v0 = vec3i_sub(b,a);
	vec3i v1 = vec3i_sub(c,a);
	vec3i v2 = vec3i_sub(p,a);
	int d00 = vec3i_dot(v0,v0);
	int d01 = vec3i_dot(v0,v1);
	int d11 = vec3i_dot(v1,v1);
	int d20 = vec3i_dot(v2,v0);
	int d21 = vec3i_dot(v2,v1);
	int denom = d00*d11 - d01*d01;
	out.y = (float)(d11*d20 - d01*d21)/(float)denom;
	out.z = (float)(d00*d21 - d01*d20)/(float)denom;
	out.x = 1.0f-(out.z+out.y);
	return out;
}
vec3f bary3carth(vec3i a, vec3i b, vec3i c, vec3f p)
{
	vec3f out = {0};
	out.x = a.x*p.x+b.x*p.y+c.x*p.z;
	out.y = a.y*p.x+b.y*p.y+c.y*p.z;
	out.z = a.z*p.x+b.z*p.y+c.z*p.z;
	return out;
}
vec2f bary2carth(vec2f a, vec2f b, vec2f c, vec3f p)
{
	vec2f out = {0};
	out.x = a.x*p.x+b.x*p.y+c.x*p.z;
	out.y = a.y*p.x+b.y*p.y+c.y*p.z;
	if(out.x < 0.0f)
		out.x = 0.0f;
	if(out.y < 0.0f)
		out.y = 0.0f;
	if(out.x > 1.0f)
		out.x = 1.0f;
	if(out.y > 1.0f)
		out.y = 1.0f;
	return out;
}
vec3f vec3f_trans(vec3f a, mat4f m)
{
	vec3f out = {0};
	float w;

	out.x = m.m0*a.x + m.m4*a.y + m.m8*a.z + m.m12;
	out.y = m.m1*a.x + m.m5*a.y + m.m9*a.z + m.m13;
	out.z = m.m2*a.x + m.m6*a.y + m.m10*a.z + m.m14;
	w = m.m3*a.x + m.m7*a.y + m.m11*a.z + m.m15;

	out.x = out.x/w;
	out.y = out.y/w;
	out.z = out.z/w;

	return out;
}
vec3f vec3f_rotate(vec3f a, vec4f q)
{
	vec3f out = {0};
    out.x = a.x*(q.x*q.x + q.w*q.w - q.y*q.y - q.z*q.z) + a.y*(2*q.x*q.y - 2*q.w*q.z) + a.z*(2*q.x*q.z + 2*q.w*q.y);
    out.y = a.x*(2*q.w*q.z + 2*q.x*q.y) + a.y*(q.w*q.w - q.x*q.x + q.y*q.y - q.z*q.z) + a.z*(-2*q.w*q.x + 2*q.y*q.z);
    out.z = a.x*(-2*q.w*q.y + 2*q.x*q.z) + a.y*(2*q.w*q.x + 2*q.y*q.z)+ a.z*(q.w*q.w - q.x*q.x - q.y*q.y + q.z*q.z);

	return out;
}
vec3f vec_project_plane(vec3f p, vec3f o, vec3f n)
{
	vec3f v = vec3f_sub(p, o);
	float dist = vec3f_dot(v, n);
	return vec3f_sub(p, vec3f_scale(n,dist));
}
vec3f vec_project_line(vec3f p, vec3f a, vec3f b)
{
	vec3f ab = vec3f_sub(b,a);
	vec3f ap = vec3f_sub(p,a);
	float abap = vec3f_dot(ab, ap);
	float len = vec3f_len2(ab);
	return vec3f_add(a, vec3f_scale(ab, abap/len));
}
vec3f vec_project_segment(vec3f p, vec3f a, vec3f b)
{
	vec3f proj = vec_project_line(p, a, b);
	return vec3f_clamp(proj, a, b);
}

float vec3f_dist_plane(vec3f p, vec3f n, vec3f a)
{
	float npdot = vec3f_dot(n, p);
	return n.x*a.x + n.y*a.y + n.z*a.z - npdot;
}

vec4f vec4f_mul(vec4f a, vec4f b)
{
	return (vec4f){a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w};
}
vec4f vec4f_from_euler(float x, float y, float z)
{
//y = attitude
//z = heading
//x = bank
    vec4f out;

    float cx = cosf(x / 2.f);
    float cy = cosf(y / 2.f);
    float cz = cosf(z / 2.f);
    float sx = sinf(x / 2.f);
    float sy = sinf(y / 2.f);
    float sz = sinf(z / 2.f);

    out.x = sz*sy*cx + cz*cy*sx;
    out.y = sz*cy*cx + cz*sy*sx;
    out.z = cz*sy*cx - sz*cy*sx;
    out.w = cz*cy*cx - sz*sy*sx;

    return out;
}

float vec2f_dist(vec2f a, vec2f b)
{
	float dx = b.x-a.x;
	float dy = b.y-a.y;
	return sqrtf(dx*dx + dy*dy);
}
vec2f vec2f_sub(vec2f a, vec2f b)
{
	return (vec2f){a.x-b.x, a.y-b.y};
}
vec2f vec2f_add(vec2f a, vec2f b)
{
	return (vec2f){a.x+b.x, a.y+b.y};
}
vec2f vec2f_div(vec2f a, vec2f b)
{
	return (vec2f){a.x/b.x, a.y/b.y};
}
vec2f vec2f_div_f(vec2f a, float b)
{
	return (vec2f){a.x/b, a.y/b};
}

void vec2i_swap(vec2i *a, vec2i *b)
{
	vec2i tmp = *a;
	*a = *b;
	*b = tmp;
}
void vec2f_swap(vec2f *a, vec2f *b)
{
	vec2f tmp = *a;
	*a = *b;
	*b = tmp;
}
void vec3i_swap(vec3i *a, vec3i *b)
{
	vec3i tmp = *a;
	*a = *b;
	*b = tmp;
}
void vec3f_swap(vec3f *a, vec3f *b)
{
	vec3f tmp = *a;
	*a = *b;
	*b = tmp;
}

vec3f vec3f_lerp(vec3f a, vec3f b, float amt)
{
	vec3f out = {0};
	out.x = lerp(a.x, b.x, amt);
	out.y = lerp(a.y, b.y, amt);
	out.z = lerp(a.z, b.z, amt);
	return out;
}
vec3i vec3i_lerp(vec3i a, vec3i b, float amt)
{
	vec3i out = {0};
	out.x = lerp_i(a.x, b.x, amt);
	out.y = lerp_i(a.y, b.y, amt);
	out.z = lerp_i(a.z, b.z, amt);
	return out;
}
vec2f vec2f_lerp(vec2f a, vec2f b, float amt)
{
	vec2f out = {0};
	out.x = lerp(a.x, b.x, amt);
	out.y = lerp(a.y, b.y, amt);
	return out;
}
vec4f vec4f_lerp(vec4f a, vec4f b, float amt)
{
       vec4f out = {0};
       out.x = lerp(a.x, b.x, amt);
       out.y = lerp(a.y, b.y, amt);
       out.z = lerp(a.z, b.z, amt);
       out.w = lerp(a.z, b.z, amt);
       return out;
}

float vec3f_lerp_plane(vec3f p, vec3f n, vec3f a, vec3f b)
{
	float npdot = vec3f_dot(n, p);
	float andot = vec3f_dot(a, n);
	float bndot = vec3f_dot(b, n);

	return (npdot-andot)/(bndot-andot);
}

vec3f vec3f_clamp(vec3f p, vec3f a, vec3f b)
{
	vec3f out;
	out.x = a.x < b.x ? clamp_f(p.x, a.x, b.x) : clamp_f(p.x, b.x, a.x);
	out.y = a.y < b.y ? clamp_f(p.y, a.y, b.y) : clamp_f(p.y, b.y, a.y);
	out.z = a.z < b.z ? clamp_f(p.z, a.z, b.z) : clamp_f(p.z, b.z, a.z);
	return out;
}

float lerp(float a, float b, float amt)
{
	return (float)a+amt*(b-a);
}
float inv_lerp(float a, float b, float c)
{
	return (float)(c-a)/(b-a);
}
float lerp_i(int a, int b, float amt)
{
	return (int)(a+amt*(b-a));
}
float inv_lerp_i(int a, int b, int c)
{
	return (float)(c-a)/(b-a);
}

float clamp_f(float a, float min, float max)
{
	if(a <= min)
		return min;
	else if(a >= max)
		return max;
	return a;
}
int clamp_i(int a, int min, int max)
{
	if(a <= min)
		return min;
	else if(a >= max)
		return max;
	return a;
}
float wrap_one_f(float a)
{
	return a - floorf(a);
}

mat4f mat_identity()
{
	mat4f out = {1.0f, 0.0f, 0.0f, 0.0f,
				 0.0f, 1.0f, 0.0f, 0.0f,
				 0.0f, 0.0f, 1.0f, 0.0f,
				 0.0f, 0.0f, 0.0f, 1.0f};
	return out;
}
mat4f mat_project(float fov, float aspect, float near, float far)
{
	float fovitan = 1.f/tanf(fov*0.5f);
	mat4f out = {0};
	out.m0 = aspect*fovitan;
	out.m5 = fovitan;
	out.m10 = far/(far-near);
	out.m11 = 1.f;
	out.m14 = (-far*near)/(far-near);
	return out;
}
mat4f mat_mul(mat4f a, mat4f b)
{
	mat4f out = {0};
	out.m0 = a.m0*b.m0 + a.m1*b.m4 + a.m2*b.m8 + a.m3*b.m12;
	out.m1 = a.m0*b.m1 + a.m1*b.m5 + a.m2*b.m9 + a.m3*b.m13;
	out.m2 = a.m0*b.m2 + a.m1*b.m6 + a.m2*b.m10 + a.m3*b.m14;
	out.m3 = a.m0*b.m3 + a.m1*b.m7 + a.m2*b.m11 + a.m3*b.m15;

	out.m4 = a.m4*b.m0 + a.m5*b.m4 + a.m6*b.m8 + a.m7*b.m12;
	out.m5 = a.m4*b.m1 + a.m5*b.m5 + a.m6*b.m9 + a.m7*b.m13;
	out.m6 = a.m4*b.m2 + a.m5*b.m6 + a.m6*b.m10 + a.m7*b.m14;
	out.m7 = a.m4*b.m3 + a.m5*b.m7 + a.m6*b.m11 + a.m7*b.m15;

	out.m8 = a.m8*b.m0 + a.m9*b.m4 + a.m10*b.m8 + a.m11*b.m12;
	out.m9 = a.m8*b.m1 + a.m9*b.m5 + a.m10*b.m9 + a.m11*b.m13;
	out.m10 = a.m8*b.m2 + a.m9*b.m6 + a.m10*b.m10 + a.m11*b.m14;
	out.m11 = a.m8*b.m3 + a.m9*b.m7 + a.m10*b.m11 + a.m11*b.m15;

	out.m12 = a.m12*b.m0 + a.m13*b.m4 + a.m14*b.m8 + a.m15*b.m12;
	out.m13 = a.m12*b.m1 + a.m13*b.m5 + a.m14*b.m9 + a.m15*b.m13;
	out.m14 = a.m12*b.m2 + a.m13*b.m6 + a.m14*b.m10 + a.m15*b.m14;
	out.m15 = a.m12*b.m3 + a.m13*b.m7 + a.m14*b.m11 + a.m15*b.m15;
	return out;
}
mat4f mat_lookat(vec3f pos, vec3f tar, vec3f up)
{
	mat4f out = {0};

	vec3f fwd = vec3f_norm(vec3f_sub(tar, pos));
	vec3f newup = vec3f_norm(vec3f_sub(up, vec3f_scale(fwd, vec3f_dot(up,fwd))));
	vec3f right = vec3f_cross(newup, fwd);

	out.m0 = right.x;
	out.m4 = right.y;
	out.m8 = right.z;
	out.m12 = -vec3f_dot(pos,right);

	out.m1 = newup.x;
	out.m5 = newup.y;
	out.m9 = newup.z;
	out.m13 = -vec3f_dot(pos,newup);

	out.m2  = fwd.x;
	out.m6  = fwd.y;
	out.m10 = fwd.z;
	out.m14 = -vec3f_dot(pos,fwd);

	out.m3 = 0.f;
	out.m7 = 0.f;
	out.m11 = 0.f;
	out.m15 = 1.f;

	return out;
}
mat4f mat_transform(vec3f pos)
{
	mat4f out = {1.0f, 0.0f, 0.0f, pos.x,
				 0.0f, 1.0f, 0.0f, pos.y,
				 0.0f, 0.0f, 1.0f, pos.z,
				 0.0f, 0.0f, 0.0f, 1.0f};
	return out;
}
mat4f mat_rotate(vec3f rot)
{
	mat4f out =  {0};

	float cosx = cosf(-rot.x);
	float sinx = sinf(-rot.x);
	float cosy = cosf(-rot.y);
	float siny = sinf(-rot.y);
	float cosz = cosf(-rot.z);
	float sinz = sinf(-rot.z);

	out.m0 = cosz*cosy;
	out.m1 = sinz*cosy;
	out.m2 = -siny;
	out.m3 = 0.f;

	out.m4 = (cosz*siny*sinx)-(sinz*cosx);
	out.m5 = (sinz*siny*sinx)+(cosz*cosx);
	out.m6 = cosy*sinx;
	out.m7 = 0.f;

	out.m8 = (cosz*siny*cosx)+(sinz*sinx);
	out.m9 = (sinz*siny*cosx)-(cosz*sinx);
	out.m10 = cosy*cosx;
	out.m11 = 0.f;

	out.m12 = 0.f;
	out.m13 = 0.f;
	out.m14 = 0.f;
	out.m15 = 1.f;

	return out;
}
mat4f mat_invert(mat4f a)
{
	mat4f out = {0};
    float b00 = a.m0*a.m5 - a.m1*a.m4;
    float b01 = a.m0*a.m6 - a.m2*a.m4;
    float b02 = a.m0*a.m7 - a.m3*a.m4;
    float b03 = a.m1*a.m6 - a.m2*a.m5;
    float b04 = a.m1*a.m7 - a.m3*a.m5;
    float b05 = a.m2*a.m7 - a.m3*a.m6;
    float b06 = a.m8*a.m13 - a.m9*a.m12;
    float b07 = a.m8*a.m14 - a.m10*a.m12;
    float b08 = a.m8*a.m15 - a.m11*a.m12;
    float b09 = a.m9*a.m14 - a.m10*a.m13;
    float b10 = a.m9*a.m15 - a.m11*a.m13;
    float b11 = a.m10*a.m15 - a.m11*a.m14;

    float invdet = 1.0f/(b00*b11 - b01*b10 + b02*b09 + b03*b08 - b04*b07 + b05*b06);

    out.m0 = (a.m5*b11 - a.m6*b10 + a.m7*b09)*invdet;
    out.m1 = (-a.m1*b11 + a.m2*b10 - a.m3*b09)*invdet;
    out.m2 = (a.m13*b05 - a.m14*b04 + a.m15*b03)*invdet;
    out.m3 = (-a.m9*b05 + a.m10*b04 - a.m11*b03)*invdet;
    out.m4 = (-a.m4*b11 + a.m6*b08 - a.m7*b07)*invdet;
    out.m5 = (a.m0*b11 - a.m2*b08 + a.m3*b07)*invdet;
    out.m6 = (-a.m12*b05 + a.m14*b02 - a.m15*b01)*invdet;
    out.m7 = (a.m8*b05 - a.m10*b02 + a.m11*b01)*invdet;
    out.m8 = (a.m4*b10 - a.m5*b08 + a.m7*b06)*invdet;
    out.m9 = (-a.m0*b10 + a.m1*b08 - a.m3*b06)*invdet;
    out.m10 = (a.m12*b04 - a.m13*b02 + a.m15*b00)*invdet;
    out.m11 = (-a.m8*b04 + a.m9*b02 - a.m11*b00)*invdet;
    out.m12 = (-a.m4*b09 + a.m5*b07 - a.m6*b06)*invdet;
    out.m13 = (a.m0*b09 - a.m1*b07 + a.m2*b06)*invdet;
    out.m14 = (-a.m12*b03 + a.m13*b01 - a.m14*b00)*invdet;
    out.m15 = (a.m8*b03 - a.m9*b01 + a.m10*b00)*invdet;

    return out;
}

mat4f mat_transpose(mat4f a)
{
    mat4f out = {0};

    out.m0 = a.m0;
    out.m1 = a.m4;
    out.m2 = a.m8;
    out.m3 = a.m12;
    out.m4 = a.m1;
    out.m5 = a.m5;
    out.m6 = a.m9;
    out.m7 = a.m13;
    out.m8 = a.m2;
    out.m9 = a.m6;
    out.m10 = a.m10;
    out.m11 = a.m14;
    out.m12 = a.m3;
    out.m13 = a.m7;
    out.m14 = a.m11;
    out.m15 = a.m15;

    return out;
}
mat4f mat_rotation(vec3f a)
{
	float cosx = cosf(-a.x);
	float sinx = cosf(-a.x);
	float cosy = cosf(-a.y);
	float siny = cosf(-a.y);
	float cosz = cosf(-a.z);
	float sinz = cosf(-a.z);

	mat4f out = {0};
    out.m0 = cosz*cosy;
    out.m4 = (cosz*siny*sinx)-(sinz*cosx);
    out.m8 = (cosz*siny*cosx)+(sinz*sinx);
    out.m12 = 0.f;

    out.m1 = sinz*cosy;
    out.m5 = (sinz*siny*sinx)+(cosz*cosx);
    out.m9 = (sinz*siny*cosx)-(cosz*sinx);
    out.m13 = 0.f;

    out.m2  = -siny;
    out.m6 	= cosy*sinx;
    out.m10 = cosy*cosx;
    out.m14 = 0.f;
	
    out.m3 = 0.f;
    out.m7 = 0.f;
    out.m11 = 0.f;
    out.m15 = 1.f;

	return out;
}

