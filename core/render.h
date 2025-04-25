#ifndef RENDER_H
#define RENDER_H
#include <windows.h>

typedef struct
{
    SHORT fps;
    LARGE_INTEGER frequency;
    LARGE_INTEGER start, end;
    double elapsedMs;
} FrameRate;

/**
 * 渲染模式
 */
typedef enum
{
    RM_BG,
    RM_CH,
    RM_EM,
    RM_CNT
} RenderMode;

typedef struct
{
    u_char r, g, b, br, bg, bb;
} RGBColor;

typedef struct
{
    wchar_t *ch;
    union
    {
        WORD attr;
        RGBColor rgb;
    } color;
} Char;

typedef struct
{
    SHORT x, y;
    SHORT w, h;
} Camera;

typedef struct
{
    SHORT w, h;
    Char **buffer;
    RenderMode mode;
    Camera camera;
} Scene;

typedef struct
{
    SHORT x, y;
    SHORT w, h;
    SHORT vx, vy;
    CHAR status;
    CHAR **data;
} Object;

FrameRate newFrameRate(SHORT fps);
void frameStart(FrameRate *fr);
void frameEnd(FrameRate *fr);
Scene *newScene(SHORT w, SHORT h);
void renderSceneByBg(Scene *scene);
void renderScene(Scene *scene);
void freeScene(Scene **scenePtr);
Object *newObject(SHORT x, SHORT y, SHORT w, SHORT h);
void rotateObject(Object *obj);
void freeObject(Object **objPtr);
void printStr(SHORT x, SHORT y, WORD color, const char *format, ...);
int getKb(void);
#endif
