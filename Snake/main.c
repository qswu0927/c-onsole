#include <stdio.h>
#include <windows.h>
#include "core/window.h"
#include "core/render.h"

Object *createWall(SHORT w, SHORT h)
{
    Object *obj = (Object *)malloc(sizeof(Object));
    obj->x = 0;
    obj->y = 0;
    obj->w = w;
    obj->h = h;
    obj->buffer = (char **)malloc(h * sizeof(char *));
    for (int i = 0; i < h; i++)
    {
        obj->buffer[i] = (char *)malloc(w * sizeof(char));
    }
    return obj;
}

void updateWall(Object *obj)
{
    for (int i = 0; i < obj->h; i++)
    {
        for (int j = 0; j < obj->w; j++)
        {
            if (i == 0 || i == obj->h - 1 || j == 0 || j == obj->w - 1)
                obj->buffer[i][j] = '#';
            else
                obj->buffer[i][j] = ' ';
        }
    }
}

int main()
{
    SHORT w = 20, h = 20;
    HANDLE hConsole = setupConsole(w, h, L"贪吃蛇");

    Object *wall = createWall(w, h);

    while (1)
    {
        updateWall(wall);
        render(wall);
        Sleep(100);
    }

    freeObject(wall);
    CloseHandle(hConsole);
    return 0;
}