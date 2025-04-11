#include <windows.h>
#include <stdio.h>
#include "render.h"

Scene *newScene(SHORT w, SHORT h)
{
    Scene *scene = (Scene *)malloc(sizeof(Scene));
    scene->w = w;
    scene->h = h;
    scene->buffer = (char **)malloc(h * sizeof(char *));
    for (int i = 0; i < h; i++)
    {
        scene->buffer[i] = (char *)malloc(w * sizeof(char));
    }
    return scene;
}

void addToScene(Scene *scene, Object *obj)
{
    for (int i = 0; i < obj->h; i++)
    {
        for (int j = 0; j < obj->w; j++)
        {
            if (i + obj->y >= 0 && i + obj->y < scene->h && j + obj->x >= 0 && j + obj->x < scene->w)
                scene->buffer[i + obj->y][j + obj->x] = obj->buffer[i][j];
        }
    }
}

void renderScene(Scene *scene)
{
    SHORT wr = 2;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD bufferSize = {wr * scene->w, scene->h};
    COORD bufferCoord = {0, 0};
    SMALL_RECT writeRegion = {0, 0, wr * scene->w - 1, scene->h - 1};
    CHAR_INFO *charInfo = (CHAR_INFO *)malloc(wr * scene->w * scene->h * sizeof(CHAR_INFO));

    for (int i = 0; i < scene->h; i++)
    {
        for (int j = 0; j < scene->w; j++)
        {
            for (int k = 0; k < wr; k++)
            {
                int index = wr * (i * scene->w + j) + k;
                charInfo[index].Char.AsciiChar = ' ';
                charInfo[index].Attributes = scene->buffer[i][j];
            }
        }
    }
    WriteConsoleOutput(hConsole, charInfo, bufferSize, bufferCoord, &writeRegion);
    free(charInfo);
}

void freeScene(Scene *scene)
{
    for (SHORT i = 0; i < scene->h; i++)
    {
        free(scene->buffer[i]);
    }
    free(scene->buffer);
    free(scene);
    scene = NULL;
}

Object *newObject(SHORT x, SHORT y, SHORT w, SHORT h)
{
    Object *obj = (Object *)malloc(sizeof(Object));
    obj->x = x;
    obj->y = y;
    obj->w = w;
    obj->h = h;
    obj->buffer = (char **)malloc(h * sizeof(char *));
    for (int i = 0; i < h; i++)
    {
        obj->buffer[i] = (char *)malloc(w * sizeof(char));
    }
    return obj;
}

void freeObject(Object *obj)
{
    for (SHORT i = 0; i < obj->h; i++)
    {
        free(obj->buffer[i]);
    }
    free(obj->buffer);
    free(obj);
    obj = NULL;
}
