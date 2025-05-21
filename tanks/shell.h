#ifndef SHELL_H
#define SHELL_H
#include "../core/util.h"
#include "../core/render.h"

#define SHELL_MAX_DAMAGE 25

struct Tank;

typedef struct Shell
{
    SHORT x, y;
    SHORT vx, vy;
    SHORT data;
    SHORT ownerId;
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
void fireShell(Shell **shellPtr, struct Tank *tank);
void updateShell(Shell **shellPtr, struct Tank **tanks, SHORT tankCount, Object *map, Scene *scene);
void setSceneShell(Scene *scene, Shell *shell);
void setSceneFrag(Scene *scene, Object *frag);
void freeShell(Shell **shellPtr);

#endif
