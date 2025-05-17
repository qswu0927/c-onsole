#include "shell.h"
#include <windows.h>
#include "../core/console.h"
#include "tank.h"

Shell *newShell(SHORT maxLen)
{
    Shell *shell = (Shell *)malloc(sizeof(Shell));
    shell->x = 0;
    shell->y = 0;
    shell->data = 0;
    shell->next = NULL;
    return shell;
}

void updateShell(Shell *shell, Tank **tanks, SHORT tankCount, Object *map)
{
}

wchar_t *getShellCh(Scene *scene, SHORT data)
{
    if (scene->mode & RM_BG)
        return _wcsdup(L"  ");
    if (scene->mode & RM_CH)
    {
        switch (data)
        {
        case 0:
            return _wcsdup(L"鸟");
        case 1:
            return _wcsdup(L"蛙");
        case 2:
            return _wcsdup(L"鱼");
        default:
            return _wcsdup(L"鼠");
        }
    }
    switch (data)
    {
    case 0:
        return _wcsdup(L"🎇");
    case 1:
        return _wcsdup(L"🚀");
    case 2:
        return _wcsdup(L"🔥");
    default:
        return _wcsdup(L"🍄");
    }
}

Rgb getShellRgb(Scene *scene, SHORT data)
{
    if (scene->mode & RM_BG)
        return Rgbb(40, 120, 0);
    if (scene->mode & RM_CH)
    {
        switch (data)
        {
        case 0:
            return Rgb(147, 112, 219);
        case 1:
            return Rgb(0, 128, 0);
        case 2:
            return Rgb(0, 191, 255);
        default:
            return Rgb(190, 190, 190);
        }
    }
    return Rgb(255, 165, 0);
}

void setSceneShell(Scene *scene, Shell *shell)
{
    for (Shell *node = shell; node != NULL; node = node->next)
    {
        scene->buffer[node->y][node->x].ch = getShellCh(scene, node->data);
        scene->buffer[node->y][node->x].color.rgb = getShellRgb(scene, node->data);
    }
}

void freeShell(Shell **shellPtr)
{
    if (!shellPtr || !*shellPtr)
        return;
    Shell *shell = *shellPtr;
    while (shell != NULL)
    {
        Shell *next = shell->next;
        free(shell);
        shell = next;
    }
    *shellPtr = NULL;
}
