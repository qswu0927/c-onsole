#ifndef RENDER_H
#define RENDER_H

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
    wchar_t *ch;
    WORD color;
    u_char rgb[6];
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
void freeObject(Object **objPtr);
void printStr(SHORT x, SHORT y, WORD color, const char *format, ...);
#endif
