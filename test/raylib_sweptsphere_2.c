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

#define COLLISION_FALSE	0;
#define COLLISION_TRUE 	1;
#define COLLISION_DONE 	2;
#define COLLISION_TRUE_1 	3;
#define COLLISION_TRUE_2 	4;

#define SMALLNR         0.000001f

typedef struct collision
{
	vec3f pos;
	vec3f vel;
	float distance2;
} collision;


float r = 1.0f;

collision col;

float vec_len(vec3f v)
{
	return sqrtf((v.x*v.x) + (v.y*v.y) + (v.z*v.z));
}
float vec_len_2(vec3f v)
{
	return ((v.x*v.x) + (v.y*v.y) + (v.z*v.z));
}
float vec_dist(vec3f a, vec3f b)
{
	float dx = b.x-a.x;
	float dy = b.y-a.y;
	float dz = b.z-a.z;
	return sqrtf(dx*dx + dy*dy + dz*dz);
}
float vec_dist_2(vec3f a, vec3f b)
{
	float dx = b.x-a.x;
	float dy = b.y-a.y;
	float dz = b.z-a.z;
	return (dx*dx + dy*dy + dz*dz);
}
float lerp(float a, float b, float amt)
{
    return (float)a+amt*(b-a);
}
float inv_lerp(float a, float b, float c)
{
    return (float)(c-a)/(b-a);
}
float clamp(float val, float min, float max)
{
	if(val < min)
		val = min;
	if(val > max)
		val = max;
	return val;
}
vec3f vec_clamp(vec3f p, vec3f a, vec3f b)
{
	vec3f out;
	out.x = a.x < b.x ? clamp(p.x, a.x, b.x) : clamp(p.x, b.x, a.x);
	out.y = a.y < b.y ? clamp(p.y, a.y, b.y) : clamp(p.y, b.y, a.y);
	out.z = a.z < b.z ? clamp(p.z, a.z, b.z) : clamp(p.z, b.z, a.z);
	return out;
}
vec3f vec_project_line(vec3f p, vec3f a, vec3f b)
{
	vec3f ab = vec_sub(b, a);
	vec3f ap = vec_sub(p, a);
	float abap = vec_dot(ab, ap);
	float len = vec_len_2(ab);

	return vec_add(a, vec_mul_f(ab, abap/len));
}
vec3f vec_project_segment(vec3f p, vec3f a, vec3f b)
{
	vec3f proj = vec_project_line(p, a, b);
	return vec_clamp(proj, a, b);
}

vec3f vec3f_lerp(vec3f a, vec3f b, float amt)
{
	vec3f out = {0};
	out.x = lerp(a.x, b.x, amt);
	out.y = lerp(a.y, b.y, amt);
	out.z = lerp(a.z, b.z, amt);
	return out;
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
int swept_tri_collision(vec3f pos, float r, vec3f vel, vec3f a, vec3f b, vec3f c, vec3f n, collision *out)
{
    vec3f pv = vec_add(pos, vel);
    float l;

    vec3f v1; 
    float d1; 
    vec3f a1 = vec_sub(a, pos);
    vec3f b1 = vec_sub(b, pos);
    vec3f c1 = vec_sub(c, pos);

    vec3f v2; 
    float d2; 
    float e2; 
    vec3f a2 = vec_sub(a, pv);
    vec3f b2 = vec_sub(b, pv);
    vec3f c2 = vec_sub(c, pv);

    v1 = vec_cross(vec_sub(b1, a1), vec_sub(c1, a1));
    d1 = vec_dot(a1, v1);

    v2 = vec_cross(vec_sub(b2, a2), vec_sub(c2, a2));
    d2 = vec_dot(a2, v2);
    e2 = vec_dot(v2, v2);

    if((d1 * d2) <= 0)
    {
        l = inv_lerp(d1, d2, 0);
        out->pos = vec3f_lerp(pos, pv, l);
        if(point_in_tri(out->pos, a, b, c))
        {
            out->pos = vec_add(out->pos, vec_mul_f(n, r));
            out->vel = vec_sub(vec_project_plane(pv, out->pos, n), out->pos);
            out->distance2 = vec_len_2(vel);
            return COLLISION_TRUE;
        }
		else
		{
			float pab = vec_dist_2(vec_project_segment(out->pos, a, b), out->pos);
			float pac = vec_dist_2(vec_project_segment(out->pos, a, c), out->pos);
			float pbc = vec_dist_2(vec_project_segment(out->pos, b, c), out->pos);
			if(pab < r*r || pac < r*r || pbc < r*r)
			{
				out->pos = vec_add(out->pos, vec_mul_f(n, r));
				out->vel = vec_sub(vec_project_plane(pv, out->pos, n), out->pos);
				out->distance2 = vec_len_2(vel);
				return COLLISION_TRUE;
			}
		}
    }
    else if((d2*d2) <= (r*r*e2))
    {
        if(point_in_tri(pv, a, b, c))
        {
            out->pos = vec_project_plane(pv, a, n);
            out->pos = vec_add(out->pos, vec_mul_f(n, r));
            out->vel = (vec3f) {0.0f, 0.0f, 0.0f};
            out->distance2 = 0.0f;
            return COLLISION_DONE;
        }
		else
		{
			float pab = vec_dist_2(vec_project_segment(pv, a, b), out->pos);
			float pac = vec_dist_2(vec_project_segment(pv, a, c), out->pos);
			float pbc = vec_dist_2(vec_project_segment(pv, b, c), out->pos);
			if(pab < r*r || pac < r*r || pbc < r*r)
			{
				out->pos = vec_project_plane(pv, a, n);
				out->pos = vec_add(out->pos, vec_mul_f(n, r));
				out->vel = (vec3f) {0.0f, 0.0f, 0.0f};
				out->distance2 = 0.0f;
				return COLLISION_TRUE_2;
			}
		}
    }
	out->pos = pv;
    out->vel = vel;
    out->distance2 = vec_len_2(vel);
    return COLLISION_FALSE;
}

vec3f player_collide(vec3f pos, vec3f vel, Mesh m)
{
	int try = 0;
	int collided = 0;
	collision col = {0};
	col.pos = pos;
	col.vel = vel;
	while(try < 1)
	{
		int vcnt = 0;
		vec3f *verts = (vec3f*)m.vertices;
		vec3f *norms = (vec3f*)m.normals;
		for(int i=0;i<m.triangleCount;i++)
		{
			vec3f a = verts[vcnt];
			vec3f n1 = norms[vcnt++];
			vec3f b = verts[vcnt];
			vec3f n2 = norms[vcnt++];
			vec3f c = verts[vcnt];
			vec3f n3 = norms[vcnt++];

			vec3f n = vec_norm(vec_add(n1, vec_add(n2, n3)));

			collided = swept_tri_collision(col.pos, 1.0f, col.vel, a, b, c, n, &col);
			if(collided)
			{
				printf("col %i\n", collided);
				DrawLine3D(a, b, RED);
				DrawLine3D(b, c, RED);
				DrawLine3D(c, a, RED);
				DrawSphere(a, 0.3f, BLUE);
				DrawSphere(b, 0.3f, BLUE);
				DrawSphere(c, 0.3f, BLUE);
			}
		}
		try++;
	}
	return col.pos;
}

int main(void)
{
    const int screenWidth = 1800;
    const int screenHeight = 1000;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(15);
	Camera camera = { { 0.0f, 10.0f, 10.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f, 0 };
	SetCameraMode(camera, CAMERA_FREE);

	Model model = LoadModel("./entrance.obj");
	Texture2D tex = LoadTexture("./tiles.png");
	model.materials[0].maps[MAP_DIFFUSE].texture = tex;

	vec3f player = (vec3f) {10.0f, 10.0f, 1.0f};
	vec3f vel= (vec3f) {0.0f, 0.0f, .0f};

    while (!WindowShouldClose())
    {
		UpdateCamera(&camera);
        BeginDrawing();
            ClearBackground(RAYWHITE);
			BeginMode3D(camera);
				DrawModel(model, Vector3Zero(), 1.0f, WHITE);
				DrawSphere(player, 1.0f, RED);
				vel = Vector3Zero();
				if(IsKeyDown(KEY_W))
					vel.x += 0.001f;
				if(IsKeyDown(KEY_S))
					vel.x -= 0.001f;
				if(IsKeyDown(KEY_A))
					vel.z += 0.001f;
				if(IsKeyDown(KEY_D))
					vel.z -= 0.001f;
				player = player_collide(player, vel, model.meshes[0]);
			EndMode3D();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
