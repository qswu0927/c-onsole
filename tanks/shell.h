#ifndef SHELL_H
#define SHELL_H
#include "../core/util.h"
#include "../core/render.h"

struct Tank;

typedef struct Shell
{
    SHORT x, y;
    SHORT vx, vy;
    SHORT data;
    struct Tank *owner;
    SHORT damage;
    struct Shell *next;
} Shell;

typedef struct
{
    Shell *shell;
    struct Tank **tanks;
    SHORT tankCount;
    Object *map;
} ExplodeParams;

Rgb getShellRgb(Scene *scene, SHORT data);
void fireShell(Shell **shellPtr, struct Tank *tank, Object *map, Scene *scene);
void updateShell(Shell **shellPtr, struct Tank **tanks, SHORT tankCount, Object *map);
void setSceneShell(Scene *scene, Shell *shell);
void setSceneFrag(Scene *scene, Object *frag);
void freeShell(Shell **shellPtr);

#endif
