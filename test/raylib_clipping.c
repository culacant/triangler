#include "raylib.h"
#include <stdio.h>

#define CLIP_NEAR 200

typedef struct v2i {
	int x;
	int y;
} v2i;

v2i p1 = {200,20};
v2i p2 = {30,200};
v2i p3 = {300,100};
v2i p4 = {200,50};
v2i p5 = {200,50};

v2i c1 = {0,CLIP_NEAR};
v2i c2 = {800,CLIP_NEAR};


float inv_lerp(int a, int b, int c)
{
	return (float)(c-a)/(b-a);
}
v2i lerp(v2i a, v2i b, float amt) 
{
	v2i out = {0};
	out.x = a.x+amt*(b.x-a.x);
	out.y = a.y+amt*(b.y-a.y);
	return out;
}
int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
		printf("%i > %i = %i\n", p1.y, p2.y, CLIP_NEAR);
		float l1 = inv_lerp(p2.y, p1.y, CLIP_NEAR);
		float l2 = inv_lerp(p2.y, p3.y, CLIP_NEAR);
		printf("%f\n", l1);
		p4 = lerp(p2, p1, l1);
		p5 = lerp(p2, p3, l2);
//		p3 = lerp(p1, p2, l1);

		p2.x = GetMouseX();
		p2.y = GetMouseY();
        BeginDrawing();

            ClearBackground(RAYWHITE);

			DrawCircle(p1.x, p1.y, 5.0f, RED);
			DrawCircle(p2.x, p2.y, 5.0f, BLUE);
			DrawCircle(p3.x, p3.y, 5.0f, PINK);
			DrawCircle(p4.x, p4.y, 5.0f, GREEN);
			DrawCircle(p5.x, p5.y, 5.0f, GREEN);

			DrawLine(p1.x, p1.y, p2.x, p2.y, PURPLE);
			DrawLine(p1.x, p1.y, p3.x, p3.y, PURPLE);
			DrawLine(p2.x, p2.y, p3.x, p3.y, PURPLE);
			DrawLine(c1.x, c1.y, c2.x, c2.y, BLACK);

        EndDrawing();
    }

    CloseWindow();        // Close window and OpenGL context

    return 0;
}
