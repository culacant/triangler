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

typedef struct vec3i 
{
	int x;
	int y;
	int z;
} vec3i;

void vec3i_swap(vec3i *a, vec3i *b)
{
    vec3i tmp = *a;
    *a = *b;
    *b = tmp;
}

void triangle_color(vec3i a, vec3i b, vec3i c)
{
    if(a.y>b.y)
    {
        vec3i_swap(&a, &b);
    }
    if(a.y>c.y)
    {
        vec3i_swap(&a, &c);
    }
    if(b.y>c.y)
    {
        vec3i_swap(&b, &c);
    }

    float stepxl;
    float curxl;
    float stepxs;
    float curxs;

    float stepzl;
    float curzl;
    float stepzs;
    float curzs;

    int miny;
    int maxy;

    for(int h=0;h<2;h++)
    {
        if(h==0)
        {
            miny = a.y;
            maxy = b.y;

            stepxl = (float)(a.x-b.x)/(float)(a.y-b.y);
            curxl = (float)a.x;
            stepxs = (float)(a.x-c.x)/(float)(a.y-c.y);
            curxs = (float)a.x;

            stepzl = (float)(a.z-b.z)/(float)(a.y-b.y);
            curzl = (float)a.z;
            stepzs = (float)(a.z-c.z)/(float)(a.y-c.y);
            curzs = (float)a.z;

        }
        else
        {
            miny = b.y;
            maxy = c.y;

            stepxl = (float)(b.x-c.x)/(float)(b.y-c.y);
            curxl = (float)b.x;
            curxs -= stepxs;

            stepzl = (float)(b.z-c.z)/(b.y-c.y);
            curzl = (float)b.z;
            curzs -= stepzs;
        }

        for(int y=miny;y<maxy;y++)
        {
            int minx = (int)curxl;
            int maxx = (int)curxs;
            float minz = curzl;
            float maxz = curzs;

            if(minx>maxx)
            {
                int tmp = minx;
                minx = maxx;
                maxx = tmp;
                float tmpf = minz;
                minz = maxz;
                maxz = tmpf;
            }

            float dz = (maxz-minz)/(float)(maxx-minx);

            for(int x = minx;x<maxx;x++)
            {
                minz += dz;
				DrawPixel(x, y, RED);
            }

            curxl += stepxl;
            curxs += stepxs;
            curzl += stepzl;
            curzs += stepzs;
        }
    }
}

vec3i vec3i_rot(vec3i a, float t, vec3i mid)
{
	vec3i out = {0};

	float x1 = a.x - mid.x;
	float y1 = a.y - mid.y;

	float x2 = x1 * cosf(t) - y1 * sinf(t);
	float y2 = x1 * sinf(t) + y1 * cosf(t);

	out.x = x2 + mid.x;
	out.y = y2 + mid.y;
	return out;
}

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;
	float angle = 0;
	float time = 0;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(60);

	vec3i ao = (vec3i){100, 200, 300};
	vec3i bo = (vec3i){200, 300, 200};
	vec3i co = (vec3i){300, 100, 100};
	vec3i a = (vec3i){100, 200, 300};
	vec3i b = (vec3i){200, 300, 200};
	vec3i c = (vec3i){300, 100, 100};
	vec3i n = (vec3i){(a.x+b.x+c.x)/3,(a.y+b.y+c.y)/3,(a.z+b.z+c.z)/3};

    while (!WindowShouldClose())
    {
			angle += 0.1f*DEG2RAD;
			a = vec3i_rot(ao, angle, n);
			b = vec3i_rot(bo, angle, n);
			c = vec3i_rot(co, angle, n);
        BeginDrawing();
            ClearBackground(RAYWHITE);
			triangle_color(a, b, c);
			DrawPixel(n.x, n.y, GREEN);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
