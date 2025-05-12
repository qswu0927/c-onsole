#include "render.h"
#include <stdio.h>
// #include <conio.h>
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

void mixColor(Rgb *dest, Rgb *src, double alpha)
{
    dest->r = (u_char)(dest->r * (1 - alpha) + src->r * alpha);
    dest->g = (u_char)(dest->g * (1 - alpha) + src->g * alpha);
    dest->b = (u_char)(dest->b * (1 - alpha) + src->b * alpha);
    dest->br = (u_char)(dest->br * (1 - alpha) + src->br * alpha);
    dest->bg = (u_char)(dest->bg * (1 - alpha) + src->bg * alpha);
    dest->bb = (u_char)(dest->bb * (1 - alpha) + src->bb * alpha);
}

void orColor(Rgb *dest, Rgb *src)
{
    dest->r |= src->r;
    dest->g |= src->g;
    dest->b |= src->b;
    dest->br |= src->br;
    dest->bg |= src->bg;
    dest->bb |= src->bb;
}

WORD rgb2attr(Rgb rgb)
{
    SHORT threshold = 50;
    SHORT threshold1 = 180;
    SHORT threshold2 = 127;
    WORD attr = 0;
    if (rgb.r > threshold)
        attr |= FOREGROUND_RED;
    if (rgb.g > threshold)
        attr |= FOREGROUND_GREEN;
    if (rgb.b > threshold)
        attr |= FOREGROUND_BLUE;
    if (rgb.r + rgb.g + rgb.b > 2 * threshold1)
        attr |= FOREGROUND_INTENSITY;
    if (attr == 0 && rgb.r + rgb.g + rgb.b > threshold2)
        attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

    WORD attrB = 0;
    if (rgb.br > threshold)
        attrB |= BACKGROUND_RED;
    if (rgb.bg > threshold)
        attrB |= BACKGROUND_GREEN;
    if (rgb.bb > threshold)
        attrB |= BACKGROUND_BLUE;
    if (rgb.br + rgb.bg + rgb.bb > 2 * threshold1)
        attrB |= BACKGROUND_INTENSITY;
    if (attrB == 0 && rgb.br + rgb.bg + rgb.bb > threshold2)
        attrB = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;

    return attr | attrB;
}

void clearScene(Scene *scene)
{
    // clearSceneP(scene, 0, 0, scene->w, scene->h);
    clearSceneP(scene, scene->camera.x, scene->camera.y, scene->camera.w, scene->camera.h);
}

void clearSceneP(Scene *scene, SHORT x, SHORT y, SHORT w, SHORT h)
{
    for (SHORT i = y; i < y + h && i < scene->h; i++)
    {
        for (SHORT j = x; j < x + w && j < scene->w; j++)
        {
            if (scene->buffer[i][j].ch != NULL)
                free(scene->buffer[i][j].ch);
            scene->buffer[i][j].ch = _wcsdup(L"  ");
            scene->buffer[i][j].color.rgb = (Rgb){0};
            scene->buffer[i][j].attrSet = false;
        }
    }
}

Scene *newScene(SHORT w, SHORT h)
{
    Scene *scene = (Scene *)malloc(sizeof(Scene));
    scene->w = w;
    scene->h = h;
    scene->mode = RM_BG;
    scene->camera = (Camera){0, 0, w, h};
    scene->buffer = (Char **)malloc(scene->h * sizeof(Char *));
    for (SHORT i = 0; i < scene->h; i++)
    {
        scene->buffer[i] = (Char *)calloc(scene->w, sizeof(Char));
    }
    clearScene(scene);
    return scene;
}

/**
 * 渲染场景
 * 使用WriteConsoleOutput整体输出
 */
void renderSceneC(Scene *scene)
{
    SHORT wr = 2;
    COORD bufferSize = {wr * scene->camera.w, scene->camera.h};
    COORD bufferCoord = {0, 0};
    SMALL_RECT writeRegion = {0, 0, wr * scene->camera.w - 1, scene->camera.h - 1};

    CHAR_INFO *charInfo = (CHAR_INFO *)malloc(wr * scene->camera.w * scene->camera.h * sizeof(CHAR_INFO));
    if (!charInfo)
        return;

    for (SHORT y = 0; y < scene->camera.h; y++)
    {
        for (SHORT srcX = 0, x = 0; srcX < scene->camera.w; srcX++)
        {
            Char cha = scene->buffer[scene->camera.y + y][scene->camera.x + srcX];
            for (SHORT i = 0; cha.ch[i] != L'\0'; i++)
            {
                SHORT index = y * wr * scene->camera.w + x;
                wchar_t ch = cha.ch[i];
                if (isWideChar(ch))
                {
                    // 双宽度字符，占用2个CHAR_INFO单元
                    charInfo[index].Char.UnicodeChar = ch;
                    charInfo[index].Attributes =
                        COMMON_LVB_LEADING_BYTE | (cha.attrSet ? cha.color.attr : rgb2attr(cha.color.rgb));

                    charInfo[index + 1].Char.UnicodeChar = L' '; // 占位
                    charInfo[index + 1].Attributes =
                        COMMON_LVB_TRAILING_BYTE;

                    x += 2;
                }
                else
                {
                    // 单宽度字符
                    charInfo[index].Char.UnicodeChar = ch;
                    charInfo[index].Attributes =
                        cha.attrSet ? cha.color.attr : rgb2attr(cha.color.rgb);
                    x += 1;
                }
            }
        }
    }
    WriteConsoleOutput(hStdOut, charInfo, bufferSize, bufferCoord, &writeRegion);
    free(charInfo);
    clearScene(scene);
}

/**
 * 渲染场景
 * 支持emoji（使用虚拟终端序列，需要控制台支持，例如Windows Terminal）
 */
void renderSceneV(Scene *scene)
{
    DWORD written;
    for (SHORT i = 0; i < scene->camera.h; i++)
    {
        SetConsoleCursorPosition(hStdOut, (COORD){0, i});
        for (SHORT j = 0; j < scene->camera.w; j++)
        {
            Char cha = scene->buffer[scene->camera.y + i][scene->camera.x + j];

            if (vtSupported)
            {
                // 预分配静态缓冲区（避免每次循环分配）
                static char colorSeq[64];
                // 直接写入（避免snprintf开销）
                int len = sprintf(colorSeq, "\x1b[38;2;%d;%d;%d;48;2;%d;%d;%dm",
                                  cha.color.rgb.r, cha.color.rgb.g, cha.color.rgb.b, cha.color.rgb.br, cha.color.rgb.bg, cha.color.rgb.bb);
                WriteConsoleA(hStdOut, colorSeq, len, &written, NULL);
            }
            else
            {
                // 回退到 16 色模式
                SetConsoleTextAttribute(hStdOut, cha.attrSet ? cha.color.attr : rgb2attr(cha.color.rgb));
            }
#if true
            // 每个坐标按实际字符数进行处理
            if (cha.ch != NULL)
            {
                size_t length = wcslen(cha.ch);
                if (length > 0)
                {
                    WriteConsoleW(hStdOut, cha.ch, (DWORD)length, &written, NULL);
                }
            }
            else
            {
                // 处理空指针情况，比如写入空格或其他占位符
                static const wchar_t space = L' ';
                WriteConsoleW(hStdOut, &space, 1, &written, NULL);
            }
#else
            // 每个坐标统一按两个字符数进行处理
            WriteConsoleW(hStdOut, cha.ch, 2, &written, NULL);
#endif
        }
    }
    // 重置颜色
    if (vtSupported)
        WriteConsoleA(hStdOut, "\x1b[0m", 4, &written, NULL);
    clearScene(scene);
}

void renderScene(Scene *scene)
{
    if (scene->mode & RM_V)
    {
        renderSceneV(scene);
        return;
    }
    renderSceneC(scene);
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
                // ch统一动态分配，需要释放
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

RenderMode changeSceneMode(Scene *scene)
{
    if (!scene->mode)
        scene->mode = RM_BG;
    if (scene->mode & RM_V)
    {
        scene->mode ^= RM_V;
        scene->mode <<= 1;
        if (scene->mode >= RM_V)
            scene->mode = RM_BG;
    }
    else
        scene->mode |= RM_V;
    return scene->mode;
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
    obj->next = NULL;
    obj->data = (CHAR **)malloc(obj->h * sizeof(CHAR *));
    for (SHORT i = 0; i < h; i++)
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
    for (SHORT i = 0; i < w; i++)
    {
        rotatedData[i] = (CHAR *)malloc(h * sizeof(CHAR));
    }

    rotate90Clockwise(h, w, obj->data, rotatedData);

    // 释放原数据
    for (SHORT i = 0; i < h; i++)
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

void setSceneStr(Scene *scene, SHORT x, SHORT y, Rgb rgb, const char *format, ...)
{
    // 使用可变参数列表来格式化字符串
    va_list args;
    va_start(args, format);

    // 动态分配缓冲区
    SHORT bufferSize = _vscprintf(format, args) + 1;
    char *buffer = (char *)malloc(bufferSize * sizeof(char));

    // 格式化字符串到动态分配的缓冲区
    vsnprintf(buffer, bufferSize, format, args);
    va_end(args);

    // 转换UTF-8字符串为宽字符
    SHORT wcharsNeeded = MultiByteToWideChar(CP_UTF8, 0, buffer, -1, NULL, 0);

    wchar_t *wbuffer = (wchar_t *)malloc(wcharsNeeded * sizeof(wchar_t));

    MultiByteToWideChar(CP_UTF8, 0, buffer, -1, wbuffer, wcharsNeeded);

    free(buffer);

    // 有未配对的半宽字符
    bool hasHalfCh = false;

    for (SHORT i = 0; wbuffer[i] != L'\0' && x < scene->w;)
    {
        // 释放旧字符
        if (scene->buffer[y][x].ch != NULL)
            free(scene->buffer[y][x].ch);

        // 检查是否是代理对（Emoji）
        bool isSurrogate = (wbuffer[i] >= 0xD800 && wbuffer[i] <= 0xDBFF);

        // 当前为半宽字符且不存在未配对的，则该位置分配两个字符
        if (wbuffer[i] <= 0x7F && !hasHalfCh)
        {
            scene->buffer[y][x].ch = (wchar_t *)malloc(3 * sizeof(wchar_t));
            scene->buffer[y][x].ch[0] = wbuffer[i++];
            // 分配的第二个字符不是半宽字符，则标记未配对
            if (wbuffer[i] > 0x7F)
                hasHalfCh = true;
            scene->buffer[y][x].ch[1] = wbuffer[i] == L'\0' ? L' ' : wbuffer[i++];
            scene->buffer[y][x].ch[2] = L'\0';
        }
        else if (isSurrogate)
        {
            // Emoji（代理对，占 2 个 wchar_t）
            scene->buffer[y][x].ch = (wchar_t *)malloc(3 * sizeof(wchar_t));
            scene->buffer[y][x].ch[0] = wbuffer[i++];
            scene->buffer[y][x].ch[1] = wbuffer[i++];
            scene->buffer[y][x].ch[2] = L'\0';
        }
        else
        {
            if (wbuffer[i] <= 0x7F)
                hasHalfCh = false;
            scene->buffer[y][x].ch = (wchar_t *)malloc(2 * sizeof(wchar_t));
            scene->buffer[y][x].ch[0] = wbuffer[i++];
            scene->buffer[y][x].ch[1] = L'\0';
        }

        orColor(&scene->buffer[y][x].color.rgb, &rgb);
        x++;
    }

    if (hasHalfCh)
    {
        // 补一个空格对齐
        scene->buffer[y][x].ch = (wchar_t *)malloc(2 * sizeof(wchar_t));
        scene->buffer[y][x].ch[0] = L' ';
        scene->buffer[y][x].ch[1] = L'\0';
        orColor(&scene->buffer[y][x].color.rgb, &rgb);
    }

    free(wbuffer);
}

/* int getKb(void)
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
} */

void getInput(InputCallback callback)
{
    if (callback == NULL)
        return;

    INPUT_RECORD irInBuf[128];
    DWORD cNumRead;

    // 检查输入（非阻塞）
    if (!PeekConsoleInput(hStdIn, irInBuf, 128, &cNumRead) || cNumRead == 0)
        return;

    // 读取输入
    if (!ReadConsoleInput(hStdIn, irInBuf, 1 /*只读取1个事件，剩下的仍然留在缓冲区*/, &cNumRead))
        return;

    // 遍历所有输入事件，触发回调
    for (DWORD i = 0; i < cNumRead; i++)
    {
        switch (irInBuf[i].EventType)
        {
        // 键盘事件
        case KEY_EVENT:
            WORD vkCode = irInBuf[i].Event.KeyEvent.wVirtualKeyCode;
            InputEvent e = {.record = &irInBuf[i], .kDown = irInBuf[i].Event.KeyEvent.bKeyDown, .vkCode = vkCode};
            if (e.kDown)
                e.vkdCode = e.vkCode;
            callback(e);
            break;
        // 鼠标事件
        case MOUSE_EVENT:
            if (mouseInput && *mouseInput)
            {
                MOUSE_EVENT_RECORD mer = irInBuf[i].Event.MouseEvent;
                InputEvent e = {.record = &irInBuf[i], .mState = mer.dwButtonState};
                callback(e);
            }
            break;
        case WINDOW_BUFFER_SIZE_EVENT:
            break;
        }
    }
}
