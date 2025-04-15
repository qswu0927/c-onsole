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

typedef struct
{
    wchar_t *ch;
    WORD color;
} Char;

typedef struct
{
    SHORT x, y;
    SHORT w, h;
    Char **buffer;
} Object;

typedef struct
{
    SHORT w, h;
    Char **buffer;
} Scene;

FrameRate newFrameRate(SHORT fps);
void frameStart(FrameRate *fr);
void frameEnd(FrameRate *fr);
Scene *newScene(SHORT w, SHORT h);
void addToScene(Scene *scene, Object *obj);
void renderSceneByBg(Scene *scene);
void renderScene(Scene *obj);
void freeScene(Scene *scene);
Object *newObject(SHORT x, SHORT y, SHORT w, SHORT h);
void freeObject(Object *obj);
void printString(SHORT x, SHORT y, WORD color, char *str);

#endif