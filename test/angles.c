#include <stdio.h>
#include <math.h>
#include "raylib.h"
typedef struct vec2f
{
	float x;
	float y;
} vec2f;
typedef struct vec3f
{
	float x;
	float y;
	float z;
} vec3f;
typedef struct vec4f
{
	float x;
	float y;
	float z;
	float w;
} vec4f;

vec3f vec3f_rotate4(vec3f a, vec4f q)
{
    vec3f out = {0};
    out.x = a.x*(q.x*q.x + q.w*q.w - q.y*q.y - q.z*q.z) + a.y*(2*q.x*q.y - 2*q.w*q.z) + a.z*(2*q.x*q.z + 2*q.w*q.y);
    out.y = a.x*(2*q.w*q.z + 2*q.x*q.y) + a.y*(q.w*q.w - q.x*q.x + q.y*q.y - q.z*q.z) + a.z*(-2*q.w*q.x + 2*q.y*q.z);
    out.z = a.x*(-2*q.w*q.y + 2*q.x*q.z) + a.y*(2*q.w*q.x + 2*q.y*q.z)+ a.z*(q.w*q.w - q.x*q.x - q.y*q.y + q.z*q.z);

    return out;
}
vec3f vec3f_rotate2(vec3f a, vec2f q)
{
    vec3f out = {0};
	out.x = sinf(q.x);
	out.y = cosf(q.x);
	out.z = sinf(q.y);
    return out;
}
vec4f vec4f_from_euler(float x, float y, float z)
{
//y = attitude
//z = heading
//x = bank
    vec4f out;

    float cx = cosf(x / 2.f);
    float cz = cosf(z / 2.f);
    float cy = cosf(y / 2.f);
    float sx = sinf(x / 2.f);
    float sz = sinf(z / 2.f);
    float sy = sinf(y / 2.f);

	out.x = sx * cy * cz - cx * sy * sz;
	out.y = cx * sy * cz + sx * cy * sz;
	out.z = cx * cy * sz - sx * sy * cz;
	out.w = cx * cy * cz + sx * sy * sz;


    return out;
}

int main()
{
	vec2f angle = (vec2f){20.f, 20.f};
	vec4f quat = vec4f_from_euler(angle.x,angle.y,  1.f);
	vec3f base = (vec3f){-1.f, 0.f, 0.f};;

	vec3f a2 = vec3f_rotate2(base, angle);
	vec3f a4 = vec3f_rotate4(base,quat);

	printf("angle: %f %f\n", angle.x, angle.y);
	printf("quat:  %f %f %f %f\n", quat.x, quat.y, quat.z, quat.w);

	printf("resa: %f %f %f\n", a2.x, a2.y, a2.z);
	printf("resq:  %f %f %f\n", a4.x, a4.y, a4.z);

    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.type = CAMERA_PERSPECTIVE;                   // Camera mode type
    SetCameraMode(camera, CAMERA_FREE); // Set a free camera mode

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
		UpdateCamera(&camera);
        BeginDrawing();
            ClearBackground(RAYWHITE);
			BeginMode3D(camera);
				DrawGrid(10, 1.f);
			EndMode3D();


        EndDrawing();
    }

    CloseWindow();        // Close window and OpenGL context

    return 0;
}
