#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <string.h>

#define CLIP_NEAR 200

#define vec3f Vector3
#define vec_add Vector3Add
#define vec_sub Vector3Subtract
#define vec_dot Vector3DotProduct
#define vec_cross Vector3CrossProduct
#define vec_mul_f Vector3Scale

vec3f a = (Vector3){-100.0f, 0.0f, -100.0f};
vec3f b = (Vector3){100.0f, 0.0f, -100.0f};
vec3f c = (Vector3){-100.0f, 0.0f, 100.0f};
vec3f n = (Vector3){0.0f, -1.0f, 0.0f};

vec3f p = (Vector3){0.0f, 0.0f, -99.0f};
vec3f vel = (Vector3){20.0f, 50.0f, 0.0f};
vec3f p2;
float r = 10.0f;

vec3f pout = (Vector3){0.0f, 0.0f, 50.0f};

float lerp(float a, float b, float amt)
{
    return (float)a+amt*(b-a);
}
float inv_lerp(float a, float b, float c)
{
    return (float)(c-a)/(b-a);
}

vec3f vec_project_plane(vec3f p, vec3f o, vec3f n)
{
	vec3f v = vec_sub(p, o);
	float dist = v.x*n.x + v.y*n.y + v.z*n.z;

	return vec_sub(p, vec_mul_f(n, dist));
}

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
    memcpy(&ix, &x, sizeof(float)); // unsigned int ix = (unsigned int&)x;
    memcpy(&iy, &y, sizeof(float));
    memcpy(&iz, &z, sizeof(float));

    return ((iz & ~(ix|iy)) & 0x80000000);      // real shit
}

int tri_swept(vec3f a, vec3f b, vec3f c, vec3f n, vec3f p, vec3f vel, vec3f *out)
{
	vec3f pv = Vector3Add(p, vel);
	float l;

	vec3f v1;
	float d1;
	float e1;
	vec3f a1 =  vec_sub(a, p);
	vec3f b1 =  vec_sub(b, p);
	vec3f c1 =  vec_sub(c, p);

	vec3f v2;
	float d2;
	float e2;
	vec3f a2 =  vec_sub(a, pv);
	vec3f b2 =  vec_sub(b, pv);
	vec3f c2 =  vec_sub(c, pv);

	v1 = vec_cross(vec_sub(b1,a1), vec_sub(c1,a1));
	d1 = vec_dot(a1, v1);
	e1 = vec_dot(v1, v1);

	v2 = vec_cross(vec_sub(b2,a2), vec_sub(c2,a2));
	d2 = vec_dot(a2, v2);
	e2 = vec_dot(v2, v2);

	if((d1 * d2) < 0)	// different signs
	{
		l = inv_lerp(d1, d2, 0);
		out->x = lerp(p.x, pv.x, l);
		out->y = lerp(p.y, pv.y, l);
		out->z = lerp(p.z, pv.z, l);
		if(point_in_tri(*out, a, b, c))
		{
			*out = vec_add(*out, n);
			return 1;
		}
	}
	out->x = pv.x;
	out->y = pv.y;
	out->z = pv.z;
	return 0;
}

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
	    p.x = GetMouseX()-300;
		p.y = GetMouseY()-300;


        BeginDrawing();
            ClearBackground(RAYWHITE);
			DrawCircle(a.x+300, a.y+300, 5.0f, BLUE);
			DrawCircle(b.x+300, b.y+300, 5.0f, BLUE);
			DrawCircle(c.x+300, c.y+300, 5.0f, BLUE);
			DrawLine(a.x+300, a.y+300, a.x+300+(n.x), a.y+300+(n.y), BLUE);


			DrawCircle(p.x+300, p.y+300, r, RED);

			p2 = vec_add(p, vel);
			if(tri_swept(a, b, c, n, p, vel, &pout))
				p2 = vec_project_plane(p2, a, n);
			DrawCircle(p2.x+300, p2.y+300, r, RED);
				
			DrawCircle(pout.x+300, pout.y+300, r, PURPLE);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
