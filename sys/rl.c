/*
                DrawPixel(x, y, (Color) { (col&0x00FF0000)>>16, // R
										  (col&0x0000ff00)>>8,  // G
										  (col&0x000000FF)>>0,  // B
										  0xff                  // A
*/
#include "raylib.h"
#include "rl.h"
#include "render.h"


int os_fb_init()
{
    RENDER_DATA.fd = 0;
    RENDER_DATA.tty = 0;
    RENDER_DATA.fb = NULL;

    RENDER_DATA.width = 800;
    RENDER_DATA.height = 600;

    InitWindow(RENDER_DATA.width, RENDER_DATA.height, "triangler");
    SetTargetFPS(60);

    RENDER_DATA.buf = calloc(RENDER_DATA.width * RENDER_DATA.height, sizeof(unsigned int));

    return 0;
}

void os_fb_blit()
{
    static int buffer_initialized = 0;
    static Texture2D buffer_gpu = { 0 };
    static Image buffer;
    if (!buffer_initialized)
    {
        buffer = GenImageColor(RENDER_DATA.width, RENDER_DATA.height, RED);
        ImageFormat(&buffer, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
        buffer_initialized = 1;
        buffer_gpu = LoadTextureFromImage(buffer);
        buffer_gpu.format = 7;
        unsigned int* i = buffer.data;
    }
    memcpy(buffer.data,RENDER_DATA.buf, sizeof(unsigned int)*RENDER_DATA.width*RENDER_DATA.height);

    UpdateTexture(buffer_gpu, RENDER_DATA.buf);

    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexture(buffer_gpu, 0, 0, WHITE);
    DrawFPS(RENDER_DATA.width - 80, 10);
    EndDrawing();
}

void os_fb_free()
{
    CloseWindow();
}
/*
    char keys[KEY_MAX/8+1];

    int fd_mouse;
    int mouseactivity;
    signed char mousedata[MOUSEBYTECNT];
    // 3 bytes:
1st byte:
    0x1: left button
    0x2: right button
    0x4: middle button
2nd byte:
    delta x
 3rd byte:
    delta y
    int mouseshow;
    int mousex;
    int mousey;
*/
int os_input_init()
{
    return 0;
}
int os_input_free()
{
    return 0;
}
int os_input_flush()
{
    static int oldmousex = 0;
    static int oldmousey = 0;
    INPUT_DATA.mouseactivity = 1;

    INPUT_DATA.mousedata[0] = 0;
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        INPUT_DATA.mousedata[0] |= 0x1;
    if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
        INPUT_DATA.mousedata[0] |= 0x2;
    if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON))
        INPUT_DATA.mousedata[0] |= 0x4;

    INPUT_DATA.mousedata[1] = GetMouseX() - oldmousex;
    INPUT_DATA.mousedata[2] = GetMouseY() - oldmousey;

    oldmousex = GetMouseX();
    oldmousey = GetMouseY();

    memset(INPUT_DATA.keys, 0, sizeof(char) * (KEY_MAX/8+1));
    for (int i = 0; i < KEY_MAX; i++)
    {
        if (IsKeyDown(i))
            INPUT_DATA.keys[i/8] |= 1 << (i%8);
    }
//    return !!(INPUT_DATA.keys[key/8] & (1<<(key%8)));
    return 0;
}

int os_timer_get()
{
    return (int)(GetTime() * 1000);
}
