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
    scene->mode = RM_BG;
    scene->buffer = (Char **)malloc(scene->h * sizeof(Char *));
    for (int i = 0; i < h; i++)
    {
        scene->buffer[i] = (Char *)malloc(scene->w * sizeof(Char));
    }
    return scene;
}

/**
 * 使用背景色渲染场景
 * 因CHAR_INFO不支持大于两字节字符
 */
void renderSceneByBg(Scene *scene)
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
                charInfo[index].Attributes = scene->buffer[i][j].color;
            }
        }
    }
    WriteConsoleOutput(hConsole, charInfo, bufferSize, bufferCoord, &writeRegion);
    free(charInfo);
}

/**
 * 渲染场景
 * 支持emoji（使用虚拟终端序列，需要控制台支持，例如Windows Terminal）
 */
void renderScene(Scene *scene)
{
    if (scene->mode == RM_BG)
        return renderSceneByBg(scene);

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written;
    for (int i = 0; i < scene->h; i++)
    {
        SetConsoleCursorPosition(hConsole, (COORD){0, i});
        for (int j = 0; j < scene->w; j++)
        {
            char colorSeq[32];
            snprintf(colorSeq, sizeof(colorSeq), "\x1b[38;2;%d;%d;%d;48;2;%d;%d;%dm",
                     (scene->buffer[i][j].color & FOREGROUND_RED) ? 255 : 0,
                     (scene->buffer[i][j].color & FOREGROUND_GREEN) ? 255 : 0,
                     (scene->buffer[i][j].color & FOREGROUND_BLUE) ? 255 : 0,
                     (scene->buffer[i][j].color & BACKGROUND_RED) ? 255 : 0,
                     (scene->buffer[i][j].color & BACKGROUND_GREEN) ? 255 : 0,
                     (scene->buffer[i][j].color & BACKGROUND_BLUE) ? 255 : 0);
            WriteConsoleA(hConsole, colorSeq, (DWORD)strlen(colorSeq), &written, NULL);
            WriteConsoleW(hConsole, scene->buffer[i][j].ch, 2, &written, NULL);
        }
    }
    // 重置颜色
    WriteConsoleA(hConsole, "\x1b[0m", 4, &written, NULL);
}

void freeScene(Scene **scenePtr)
{
    if (!scenePtr || !*scenePtr)
        return;

    Scene *scene = *scenePtr;
    if (scene->buffer)
    {
        for (SHORT i = 0; i < scene->h; i++)
        {
            if (scene->buffer[i])
            {
                // ch默认是指向静态字符串或常量字符串的指针，不应释放；若是动态分配则需要释放
                // free(scene->buffer[i]->ch);
                free(scene->buffer[i]);
            }
        }
        free(scene->buffer);
    }
    free(scene);
    // 通过双重指针参数置空调用方的指针
    *scenePtr = NULL;
}

Object *newObject(SHORT x, SHORT y, SHORT w, SHORT h)
{
    Object *obj = (Object *)malloc(sizeof(Object));
    obj->x = x;
    obj->y = y;
    obj->w = w;
    obj->h = h;
    obj->data = (CHAR **)malloc(obj->h * sizeof(CHAR *));
    for (int i = 0; i < h; i++)
    {
        obj->data[i] = (CHAR *)malloc(obj->w * sizeof(CHAR));
    }
    return obj;
}

void freeObject(Object **objPtr)
{
    if (!objPtr || !*objPtr)
        return;

    Object *obj = *objPtr;
    if (obj->data)
    {
        for (SHORT i = 0; i < obj->h; i++)
        {
            if (obj->data[i])
            {
                free(obj->data[i]);
            }
        }
        free(obj->data);
    }
    free(obj);
    // 通过双重指针参数置空调用方的指针
    *objPtr = NULL;
}

void printString(SHORT x, SHORT y, WORD color, char *str)
{
    COORD coord = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
    printf("%s", str);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}