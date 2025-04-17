#ifndef RENDER_H
#define RENDER_H

// "●◼◻⊙★○"

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
} Char;

typedef struct
{
    SHORT w, h;
    Char **buffer;
    RenderMode mode;
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
void renderScene(Scene *obj);
void freeScene(Scene **scenePtr);
Object *newObject(SHORT x, SHORT y, SHORT w, SHORT h);
void freeObject(Object **objPtr);
void printString(SHORT x, SHORT y, WORD color, char *str);

#endif