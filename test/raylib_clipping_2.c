#include "raylib.h"
#include "raymath.h"
#include <stdio.h>

#define vec3f Vector3
#define vec3f_add Vector3Add
#define vec3f_sub Vector3Subtract
#define vec3f_dot Vector3DotProduct
#define vec3f_cross Vector3CrossProduct
#define vec3f_mul_f Vector3Scale
#define vec3f_norm Vector3Normalize


#define CLIP_NEAR 200

typedef struct render_triangle
{
	vec3f a;
	vec3f b;
	vec3f c;
} render_triangle;

float lerp(float a, float b, float amt)
{
    return (float)a+amt*(b-a);
}
float inv_lerp(float a, float b, float c)
{
    return (float)(c-a)/(b-a);
}

vec3f vec3f_lerp(vec3f a, vec3f b, float amt)
{
    vec3f out = {0};
    out.x = lerp(a.x, b.x, amt);
    out.y = lerp(a.y, b.y, amt);
    out.z = lerp(a.z, b.z, amt);
    return out;
}

float vec3f_dist_plane(vec3f p, vec3f n, vec3f a)
{
    float npdot = vec3f_dot(n, p);
    return n.x*a.x + n.y*a.y + n.z*a.z - npdot;
}
float vec3f_lerp_plane(vec3f p, vec3f n, vec3f a, vec3f b)
{
    float npdot = vec3f_dot(n, p);
    float andot = vec3f_dot(a, n);
    float bndot = vec3f_dot(b, n);
	printf("npdot %f - andot %f / bndot %f - andot %f\n", npdot, andot, bndot, andot);

    return (npdot-andot)/(bndot-andot);
}

void draw_plane(vec3f p, vec3f n)
{
	DrawSphere(vec3f_add(p,n), 0.2f, BLUE);	
	DrawSphere(vec3f_add(p,n), 0.2f, BLUE);	
}
void draw_plane_2d(vec3f p, vec3f n)
{
	DrawLine(0, (int)p.z, 800, (int)p.z, PURPLE);
	DrawLine((int)p.x, (int)p.z, (int)(p.x+n.x*50), (int)(p.z+n.z*50), BLUE);
}
int triangle_clip_plane(vec3f p, vec3f n, render_triangle in, render_triangle *out0,render_triangle *    out1)
{
    int incnt = 0;
    int outcnt = 0;
    vec3f inp[3];
    vec3f outp[3];

    float dista = vec3f_dist_plane(p, n, in.a);
    float distb = vec3f_dist_plane(p, n, in.b);
    float distc = vec3f_dist_plane(p, n, in.c);

	if(dista <= 0 && distb <= 0 && distc <= 0)
		return 0;
	else if(dista <= 0)
	{
		if(distb <= 0)
		{
			// double: in1, out1, out2
			// clip double c, a, b
			float l1 = inv_lerp(dista, distc, 0.f);
			float l2 = inv_lerp(distb, distc, 0.f);
			out0->a = vec3f_lerp(in.a, in.c, l1);
			out0->b = vec3f_lerp(in.b, in.c, l2);
			out0->c = in.c;
			return 1;
		}
		else if(distc <= 0)
		{
			// clip double b, a, c
			float l1 = inv_lerp(dista, distb, 0.f);
			float l2 = inv_lerp(distc, distb, 0.f);
			out0->a = vec3f_lerp(in.a, in.b, l1);
			out0->b = vec3f_lerp(in.c, in.b, l2);
			out0->c = in.b;
			return 1;
		}
		else
		{
			// clip single b, c, a
			float l1 = inv_lerp(dista, distb, 0.f);
			float l2 = inv_lerp(dista, distc, 0.f);
			out0->a = vec3f_lerp(in.a, in.b, l1);
			out0->b = in.b;
			out0->c = in.c;

			out1->a = vec3f_lerp(in.a, in.c, l2);
			out1->b = out0->a;
			out1->c = in.c;
			return 2;
		}
	}
	else if(distb <= 0)
	{
		if(distc <= 0)
		{
			// clip double a, b, c
			float l1 = inv_lerp(distb, dista, 0.f);
			float l2 = inv_lerp(distc, dista, 0.f);
			out0->a = vec3f_lerp(in.b, in.a, l1);
			out0->b = vec3f_lerp(in.c, in.a, l2);
			out0->c = in.a;
			return 1;
		}
		else
		{
			// clip single a, c, b
			float l1 = inv_lerp(distb, dista, 0.f);
			float l2 = inv_lerp(distb, distc, 0.f);
			out0->a = vec3f_lerp(in.b, in.a, l1);
			out0->b = in.a;
			out0->c = in.c;

			out1->a = vec3f_lerp(in.b, in.c, l2);
			out1->b = out0->a;
			out1->c = in.c;
			return 2;
		}
	}
	else if(distc <= 0)
	{
		// clip single a, b, c
		float l1 = inv_lerp(distc, dista, 0.f);
		float l2 = inv_lerp(distc, distb, 0.f);
		out0->a = vec3f_lerp(in.c, in.a, l1);
		out0->b = in.a;
		out0->c = in.b;

		out1->a = vec3f_lerp(in.c, in.b, l2);
		out1->b = out0->a;
		out1->c = in.b;
	
		return 2;
	}
	//copy all
	*out0 = in;
	return 1;

}



int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

	vec3f a = (vec3f){0.f, 0.f, 0.f};
	vec3f b = (vec3f){0.f, 0.f, 0.f};
	vec3f c = (vec3f){0.f, 0.f, 0.f};

	vec3f a1 = (vec3f){0.f, 0.f, 0.f};

	vec3f p = (vec3f){400.f, 0.f, 200.f};
	vec3f n = (vec3f){0.f, 0.f, -1.f};

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

	int click = 0;

	render_triangle in;
	in.a = a;
	in.b = b;
	in.c = c;
	render_triangle out0;
	render_triangle out1;
	int outcnt;

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
		if(IsKeyReleased(KEY_SPACE))
		{
			float l1 = vec3f_lerp_plane(p, n, a, b);
			a1 = vec3f_lerp(a, b, l1);
			in.a = a;
			in.b = b;
			in.c = c;
			outcnt = triangle_clip_plane(p, n, in, &out0,&out1);
			switch(outcnt)
			{
				case 3:
					printf("ALL\n");
					break;
				case 2:
					printf("0:\n");
					printf("%f %f %f\n",out0.a.x, out0.a.y, out0.a.z);
					printf("%f %f %f\n",out0.b.x, out0.b.y, out0.b.z);
					printf("%f %f %f\n",out0.c.x, out0.c.y, out0.c.z);
					printf("1:\n");
					printf("%f %f %f\n",out1.a.x, out1.a.y, out1.a.z);
					printf("%f %f %f\n",out1.b.x, out1.b.y, out1.b.z);
					printf("%f %f %f\n",out1.c.x, out1.c.y, out1.c.z);
					break;
				case 1:
					printf("0:\n");
					printf("%f %f %f\n",out0.a.x, out0.a.y, out0.a.z);
					printf("%f %f %f\n",out0.b.x, out0.b.y, out0.b.z);
					printf("%f %f %f\n",out0.c.x, out0.c.y, out0.c.z);
					break;
				case 0:
				default:
					printf("NONE\n");
				break;
			}
		}
		if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
		{
			click++;
			click = click%3;
			switch(click)
			{
				case 0:
					a.x = GetMouseX();
					a.z = GetMouseY();
					break;
				case 1:
					b.x = GetMouseX();
					b.z = GetMouseY();
					break;
				default:
					c.x = GetMouseX();
					c.z = GetMouseY();
					break;
			}
		}
        BeginDrawing();

            ClearBackground(RAYWHITE);
			DrawCircle(a.x, a.z, 1.0f, RED);
			DrawCircle(b.x, b.z, 1.0f, RED);
			DrawCircle(c.x, c.z, 1.0f, RED);

			if(outcnt == 1)
			{
	DrawLine(out0.a.x, out0.a.z, out0.b.x, out0.b.z, GREEN);
	DrawLine(out0.b.x, out0.b.z, out0.c.x, out0.c.z, GREEN);
	DrawLine(out0.c.x, out0.c.z, out0.a.x, out0.a.z, GREEN);
			}
			if(outcnt == 2)
			{
	DrawLine(out0.a.x, out0.a.z, out0.b.x, out0.b.z, GREEN);
	DrawLine(out0.b.x, out0.b.z, out0.c.x, out0.c.z, GREEN);
	DrawLine(out0.c.x, out0.c.z, out0.a.x, out0.a.z, GREEN);

	DrawLine(out1.a.x, out1.a.z, out1.b.x, out1.b.z, BLUE);
	DrawLine(out1.b.x, out1.b.z, out1.c.x, out1.c.z, BLUE);
	DrawLine(out1.c.x, out1.c.z, out1.a.x, out1.a.z, BLUE);
			}
draw_plane_2d(p, n);

        EndDrawing();
    }

    CloseWindow();        // Close window and OpenGL context

    return 0;
}
