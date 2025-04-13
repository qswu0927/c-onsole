#include <windows.h>
#include <stdio.h>
#include "render.h"

FrameRate newFrameRate(SHORT fps)
{
    FrameRate fr = {.fps = fps};
    // 获取计时器的频率(每秒计数次数)，这个频率是固定的，在系统启动时确定
    QueryPerformanceFrequency(&fr.frequency);
    return fr;
}

void frameStart(FrameRate *fr)
{
    // 记录帧开始时间
    // 获取计时器当前的计数值
    QueryPerformanceCounter(&fr->start);
}

void frameEnd(FrameRate *fr)
{
    // 记录帧结束时间
    QueryPerformanceCounter(&fr->end);

    // 计算本帧耗时(毫秒)
    fr->elapsedMs = (double)(fr->end.QuadPart - fr->start.QuadPart) * 1000.0 / fr->frequency.QuadPart;

    // 计算需要睡眠的时间
    double sleepTime = (1000.0 / fr->fps) - fr->elapsedMs;
    if (sleepTime > 0)
    {
        Sleep(sleepTime);
    }

    // 重新获取时间计算实际帧时间(包括睡眠)
    QueryPerformanceCounter(&fr->end);
    fr->elapsedMs = (double)(fr->end.QuadPart - fr->start.QuadPart) * 1000.0 / fr->frequency.QuadPart;
}

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

void printString(SHORT x, SHORT y, WORD color, char *str)
{
    COORD coord = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
    printf("%s", str);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}