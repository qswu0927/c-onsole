#ifndef SHELL_H
#define SHELL_H
#include "../core/util.h"
#include "../core/render.h"

struct Tank;

typedef struct Shell
{
    SHORT x, y;
    SHORT data;
    struct Shell *next;
} Shell;

wchar_t *getShellCh(Scene *scene, SHORT data);
Rgb getShellRgb(Scene *scene, SHORT data);
Shell *newShell(SHORT maxLen);
void updateShell(Shell *shell, struct Tank **tanks, SHORT tankCount, Object *map);
void setSceneShell(Scene *scene, Shell *shell);
void freeShell(Shell **shellPtr);

#endif
