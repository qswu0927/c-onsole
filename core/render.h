#ifndef RENDER_H
#define RENDER_H
#include <windows.h>
#include <stdbool.h>

#define Rgb(r, g, b) ((Rgb){r, g, b, 0, 0, 0})
#define Rgbb(r, g, b) ((Rgb){0, 0, 0, r, g, b})

typedef void (*InputCallback)(INPUT_RECORD *inputRecord, WORD vkCode, DWORD mState);

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
    RM_BG = 1,
    RM_CH = 1 << 1,
    RM_EM = 1 << 2,
    RM_V = 1 << 3,
} RenderMode;

typedef struct
{
    u_char r, g, b, br, bg, bb;
} Rgb;

typedef struct
{
    wchar_t *ch;
    union
    {
        WORD attr;
        Rgb rgb;
    } color;
    bool attrSet;
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

typedef struct Object
{
    SHORT x, y;
    SHORT w, h;
    SHORT vx, vy;
    CHAR status;
    CHAR **data;
    struct Object *next;
} Object;

FrameRate newFrameRate(SHORT fps);
void frameStart(FrameRate *fr);
void frameEnd(FrameRate *fr);
void mixColor(Rgb *dest, Rgb *src, double alpha);
void orColor(Rgb *dest, Rgb *src);
void clearScene(Scene *scene);
void clearSceneP(Scene *scene, SHORT x, SHORT y, SHORT w, SHORT h);
Scene *newScene(SHORT w, SHORT h);
void renderScene(Scene *scene);
void freeScene(Scene **scenePtr);
Object *newObject(SHORT x, SHORT y, SHORT w, SHORT h);
void rotateObject(Object *obj);
void freeObject(Object **objPtr);
void setSceneStr(Scene *scene, SHORT x, SHORT y, Rgb rgb, const char *format, ...);
// int getKb(void);
void getInput(InputCallback callback);

#endif
