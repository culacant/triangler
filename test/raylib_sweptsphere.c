#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define vec3f Vector3
#define vec_add Vector3Add
#define vec_sub Vector3Subtract
#define vec_dot Vector3DotProduct
#define vec_cross Vector3CrossProduct
#define vec_mul_f Vector3Scale
#define vec_norm Vector3Normalize

vec3f a = (vec3f){0,0,0};
vec3f b = (vec3f){0,0,0};
vec3f c = (vec3f){0,0,0};
vec3f n = (Vector3){0.0f, -1.0f, 0.0f};

float r = 1.0f;

vec3f pout = (Vector3){0.0f, 0.0f, 0.0f};

float vec_len(vec3f v)
{
	return sqrtf((v.x*v.x) + (v.y*v.y) + (v.z*v.z));
}
float vec_dist(vec3f a, vec3f b)
{
	float dx = b.x-a.x;
	float dy = b.y-a.y;
	float dz = b.z-a.z;
	return sqrtf(dx*dx + dy*dy + dz*dz);
}
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
	float dist = vec_dot(v, n);
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


#define THIRD 0.333333
int tri_swept(vec3f a, vec3f b, vec3f c, vec3f n, vec3f p, float r, vec3f *vel, vec3f *pout)
{
	vec3f center = vec_mul_f(vec_add(a, vec_add(b, c)), THIRD);
	
	float l1 = vec_dist(a, center);
	float l2 = ((l1+r)/l1);

	vec3f aadj = vec_add(center, vec_mul_f(vec_sub(a,center), l2));
	vec3f badj = vec_add(center, vec_mul_f(vec_sub(b,center), l2));
	vec3f cadj = vec_add(center, vec_mul_f(vec_sub(c,center), l2));

	vec3f pv = Vector3Add(p, *vel);
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

	if((d1 * d2) <= 0)
	{
		l = inv_lerp(d1, d2, 0);
		pout->x = lerp(p.x, pv.x, l);
		pout->y = lerp(p.y, pv.y, l);
		pout->z = lerp(p.z, pv.z, l);
		if(point_in_tri(*pout, aadj, badj, cadj))
		{
			*pout = vec_add(*pout, vec_mul_f(n, r));
			*vel = vec_sub(vec_project_plane(pv, *pout, n), *pout);
			return 1;
		}
	}
	if((d2*d2) <= (r*r*e2))
	{
		if(point_in_tri(pv, aadj, badj, cadj))
		{
			l = inv_lerp(d1, d2, 0);
			pout->x = lerp(p.x, pv.x, l);
			pout->y = lerp(p.y, pv.y, l);
			pout->z = lerp(p.z, pv.z, l);
		
			*pout = vec_add(*pout, vec_mul_f(n,r));
			*vel = (vec3f){0.0f, 0.0f, 0.0f};
			return 2;
		}
	}
	/*
	if((d1*d1) <= ((r*r)*e1))
	{
		if(point_in_tri(p, aadj, badj, cadj))
		{
			l = inv_lerp(d1, d2, 0);
			pout->x = lerp(p.x, pv.x, l);
			pout->y = lerp(p.y, pv.y, l);
			pout->z = lerp(p.z, pv.z, l);
		
			*pout = vec_add(*pout, vec_mul_f(n,r));
			*vel = vec_sub(vec_project_plane(pv, *pout, n), *pout);
			return 1;
		}
	}
	*/
		
	*pout = pv;
	return 0;
}

vec3f p = (Vector3){0.0f, 0.0f, 0.0f};
vec3f vel = (Vector3){0.0f, 0.0f, 0.0f};
int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(60);
	Camera camera = { { 0.0f, 10.0f, 10.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f, 0 };
	SetCameraMode(camera, CAMERA_FREE);
Ray ray;
vec3f p4;
    while (!WindowShouldClose())
    {
		UpdateCamera(&camera);
		vec3f p2;
		if(IsKeyPressed(KEY_R))
		{
			a.x = rand()%10;
			a.y = rand()%10;
			a.z = rand()%10;
			b.x = rand()%10;
			b.y = rand()%10;
			b.z = rand()%10;
			c.x = rand()%10;
			c.y = rand()%10;
			c.z = rand()%10;
			n = vec_norm(vec_cross(vec_sub(b,a), vec_sub(c,a)));
p4 = vec_project_plane(camera.position, a, n);
		}
        BeginDrawing();
            ClearBackground(RAYWHITE);
			BeginMode3D(camera);
				DrawGizmo((vec3f){0,0,0});
				DrawLine3D(a,b,BLUE);
				DrawLine3D(b,c,BLUE);
				DrawLine3D(a,c,BLUE);

		if(IsKeyPressed(KEY_SPACE))
		{
			p = camera.position;
			ray = GetMouseRay((Vector2){screenWidth/2, screenHeight/2}, camera);
			vel = vec_mul_f(ray.direction, 5);
			p2 = vec_add(p, vel);
			if(tri_swept(a, b, c, n, p, r, &vel, &pout))
				p2 = vec_add(pout, vel);
			else
				p2 = vec_add(p, vel);
		}
#define HALFRED CLITERAL(Color){255,0,0,128}
#define HALFGREEN CLITERAL(Color){0,255,0,128}
#define HALFBLUE CLITERAL(Color){0,0,255,128}
vec3f p3 = vec_add(p, vec_mul_f(ray.direction,5));
DrawLine3D(p, p3, RED);
DrawSphere(p4, r, RED);




			DrawSphere(p, r, HALFRED);
			DrawSphere(p2, r/2, HALFGREEN);
			DrawSphere(pout, r, HALFBLUE);
			DrawLine3D(p, pout, RED);
			DrawLine3D(pout, p2, RED);

			//DrawLine(p.x+300, p.y+300, pout.x+300, pout.y+300, RED);
			EndMode3D();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
