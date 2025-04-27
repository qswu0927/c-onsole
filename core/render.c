#include "render.h"
#include <stdio.h>
#include <conio.h>
#include "util.h"
#include "console.h"

FrameRate newFrameRate(SHORT fps)
{
    FrameRate fr = {.fps = fps};
    // 初始化，避免首次显示无穷
    fr.elapsedMs = 1000.0 / fr.fps;
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
    scene->camera = (Camera){0, 0, w, h};
    scene->buffer = (Char **)malloc(scene->h * sizeof(Char *));
    for (int i = 0; i < scene->h; i++)
    {
        scene->buffer[i] = (Char *)malloc(scene->w * sizeof(Char));
    }

    for (int i = 0; i < scene->h; i++)
    {
        for (int j = 0; j < scene->w; j++)
        {
            scene->buffer[i][j].ch = NULL;
        }
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
    COORD bufferSize = {wr * scene->camera.w, scene->camera.h};
    COORD bufferCoord = {0, 0};
    SMALL_RECT writeRegion = {0, 0, wr * scene->camera.w - 1, scene->camera.h - 1};
    CHAR_INFO *charInfo = (CHAR_INFO *)malloc(wr * scene->camera.w * scene->camera.h * sizeof(CHAR_INFO));

    for (int i = 0; i < scene->camera.h; i++)
    {
        for (int j = 0; j < scene->camera.w; j++)
        {
            for (int k = 0; k < wr; k++)
            {
                int index = wr * (i * scene->camera.w + j) + k;
                charInfo[index].Char.AsciiChar = ' ';
                charInfo[index].Attributes = scene->buffer[scene->camera.y + i][scene->camera.x + j].color.attr;
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
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // 如果不支持虚拟终端序列
    if (!vtSupported)
    {
        renderSceneByBg(scene);
        return;
    }

    DWORD written;
    for (int i = 0; i < scene->camera.h; i++)
    {
        SetConsoleCursorPosition(hConsole, (COORD){0, i});
        for (int j = 0; j < scene->camera.w; j++)
        {
            RGBColor rgb = scene->buffer[scene->camera.y + i][scene->camera.x + j].color.rgb;

            // 预分配静态缓冲区（避免每次循环分配）
            static char colorSeq[64];

            // 直接写入（避免snprintf开销）
            int len = sprintf(colorSeq, "\x1b[38;2;%d;%d;%d;48;2;%d;%d;%dm",
                              rgb.r, rgb.g, rgb.b, rgb.br, rgb.bg, rgb.bb);
            WriteConsoleA(hConsole, colorSeq, len, &written, NULL);
#if true
            wchar_t *wstr = scene->buffer[scene->camera.y + i][scene->camera.x + j].ch;

            if (wstr != NULL)
            {
                size_t length = wcslen(wstr);
                if (length > 0)
                {
                    WriteConsoleW(hConsole, wstr, (DWORD)length, &written, NULL);
                }
            }
            else
            {
                // 处理空指针情况，比如写入空格或其他占位符
                static const wchar_t space = L' ';
                WriteConsoleW(hConsole, &space, 1, &written, NULL);
            }
#else
            WriteConsoleW(hConsole, scene->buffer[scene->camera.y + i][scene->camera.x + j].ch, 2, &written, NULL);
#endif
        }
    }
    // 重置颜色
    WriteConsoleA(hConsole, "\x1b[0m", 4, &written, NULL);
#if true
    // 重置场景缓冲区
    for (int i = 0; i < scene->camera.h; i++)
    {
        for (int j = 0; j < scene->camera.w; j++)
        {
            scene->buffer[scene->camera.y + i][scene->camera.x + j].ch = _wcsdup(L"  ");
            scene->buffer[scene->camera.y + i][scene->camera.x + j].color.rgb = (RGBColor){0, 0, 0, 0, 0, 0};
        }
    }
#endif
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
                free(scene->buffer[i]->ch);
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
    obj->vx = 0;
    obj->vy = 0;
    obj->status = 0;
    obj->data = (CHAR **)malloc(obj->h * sizeof(CHAR *));
    for (int i = 0; i < h; i++)
    {
        obj->data[i] = (CHAR *)malloc(obj->w * sizeof(CHAR));
    }
    return obj;
}

void rotateObject(Object *obj)
{
    SHORT w = obj->w;
    SHORT h = obj->h;
    CHAR **rotatedData = (CHAR **)malloc(w * sizeof(CHAR *));
    for (int i = 0; i < w; i++)
    {
        rotatedData[i] = (CHAR *)malloc(h * sizeof(CHAR));
    }

    rotate90Clockwise(h, w, obj->data, rotatedData);

    // 释放原数据
    for (int i = 0; i < h; i++)
    {
        free(obj->data[i]);
    }
    free(obj->data);

    obj->data = rotatedData;
    obj->w = h;
    obj->h = w;
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

void printStr(SHORT x, SHORT y, WORD color, const char *format, ...)
{
    // 获取标准输出句柄
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // 设置光标位置
    SetConsoleCursorPosition(hConsole, (COORD){x, y});

    // 设置文本颜色
    SetConsoleTextAttribute(hConsole, color);

    // 使用可变参数列表来格式化字符串
    va_list args;
    va_start(args, format);

    // 动态分配缓冲区
    int bufferSize = _vscprintf(format, args) + 1; // _vscprintf计算格式化字符串所需的大小
    char *buffer = (char *)malloc(bufferSize * sizeof(char));
    if (buffer == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        va_end(args);
        return;
    }

    // 格式化字符串到动态分配的缓冲区
    vsnprintf(buffer, bufferSize, format, args);

    // 输出格式化后的字符串
    printf("%s", buffer);

    // 获取控制台宽度
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    SHORT consoleWidth = csbi.dwSize.X;

    // 计算剩余宽度并填充空白字符
    int remainingWidth = consoleWidth - (x + strlen(buffer));
    for (int i = 0; i < remainingWidth; i++)
        printf(" ");

    // 释放动态分配的内存
    free(buffer);

    // 恢复默认文本颜色
    SetConsoleTextAttribute(hConsole, 7);

    // 结束可变参数列表
    va_end(args);
}

int getKb(void)
{
    if (!_kbhit())
        return -1;

    int ch = _getch();
    if (ch == 0xE0 || ch == 0)
    {
        // 是扩展键（如方向键），读取第二个字节
        int ext = _getch();
        // 返回组合值（如 0xE04D）
        return (ch << 8) | ext;
    }
    // 普通键，转大写
    return toupper(ch);
}
