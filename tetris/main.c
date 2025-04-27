#include <time.h>
#include <signal.h>
#include "../core/util.h"
#include "../core/console.h"
#include "../core/render.h"
#include "../core/music.h"
#include "map.h"
#include "block.h"

typedef struct
{
    RenderMode mode;
    int maxScore;
} Config;

bool isPaused = false;
bool isQuit = false;
Config config = {.mode = RM_BG, .maxScore = 0};

void loadConfig(const char *filename, Config *config)
{
    FILE *file = safeFopen(filename, "rb");
    if (!file)
        return;
    fread(config, sizeof(Config), 1, file);
    fclose(file);
}

void saveConfig(const char *filename, Config *config)
{
    FILE *file = safeFopen(filename, "wb");
    fwrite(config, sizeof(Config), 1, file);
    fclose(file);
}

void quit(void)
{
    saveConfig("config.dat", &config);
}

void handleSigint(int _)
{
    quit();
}

BOOL WINAPI handleConsole(DWORD event)
{
    if (event == CTRL_C_EVENT || event == CTRL_CLOSE_EVENT)
    {
        quit();
        return TRUE;
    }
    return FALSE;
}

void init(void)
{
    // 程序正常退出（main 返回或调用 exit()）
    atexit(quit);
    // 按 Ctrl+C 终止程序
    signal(SIGINT, handleSigint);
    // Windows控制台关闭、注销、关机等
    SetConsoleCtrlHandler(handleConsole, TRUE);
    loadConfig("config.dat", &config);
    srand((unsigned int)time(NULL));
}

void updateStrToScene(Scene *scene, SHORT x, SHORT y, const char *format, ...)
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

#if true
    // 统计半宽字符数，半宽字符只占一半宽度，为了不影响该行后面内容的渲染位置，调整最后一个ch分配的字符数
    SHORT halfWidthCount = 0;
    for (SHORT i = 0; wbuffer[i] != L'\0' && x < scene->w; i++)
        if (wbuffer[i] <= 0x7F)
            halfWidthCount++;

    wchar_t lastChar = L'\0';
    if (halfWidthCount % 2 == 1)
    {
        SHORT y1 = y + wcharsNeeded - halfWidthCount / 2 - 2;
        if (scene->buffer[y1][x].ch != NULL)
            lastChar = scene->buffer[y1][x].ch[0];
    }

    for (SHORT i = 0; wbuffer[i] != L'\0' && x < scene->w;)
    {
        // 释放旧字符内存
        if (scene->buffer[y][x].ch != NULL)
            free(scene->buffer[y][x].ch);

        if (i < wcharsNeeded - halfWidthCount / 2 - 2)
        {
            scene->buffer[y][x].ch = (wchar_t *)malloc(2 * sizeof(wchar_t));
            scene->buffer[y][x].ch[0] = wbuffer[i++];
            scene->buffer[y][x].ch[1] = L'\0';
        }
        else
        {
            SHORT lastChLen = halfWidthCount / 2 + 1;
            scene->buffer[y][x].ch = (wchar_t *)malloc((lastChLen + (lastChar == L'\0' ? 1 : 2)) * sizeof(wchar_t));
            for (int j = 0; j < lastChLen; j++)
            {
                scene->buffer[y][x].ch[j] = wbuffer[i++];
            }
            if (lastChar == L'\0')
            {
                scene->buffer[y][x].ch[lastChLen] = L'\0';
            }
            else
            {
                scene->buffer[y][x].ch[lastChLen] = lastChar;
                scene->buffer[y][x].ch[lastChLen + 1] = L'\0';
            }
        }

        scene->buffer[y][x].color.rgb.r |= 0xFF;
        scene->buffer[y][x].color.rgb.g |= 0xFF;

        x++;
    }
#else
    // 计算实际能写入的字符数
    int maxChars = scene->w - x;
    int charsToCopy = wcslen(wbuffer);
    if (charsToCopy > maxChars)
        charsToCopy = maxChars;

    // 释放旧字符内存
    if (scene->buffer[y][x].ch != NULL)
        free(scene->buffer[y][x].ch);

    // 分配新的内存
    scene->buffer[y][x].ch = (wchar_t *)malloc((charsToCopy + 1) * sizeof(wchar_t));

    // 拷贝内容并确保以null结尾
    wcsncpy_s(scene->buffer[y][x].ch, charsToCopy + 1, wbuffer, charsToCopy);
    scene->buffer[y][x].ch[charsToCopy] = L'\0';
    scene->buffer[y][x].color.rgb = (RGBColor){255, 255, 0, 0, 0, 0};
#endif

    free(wbuffer);
}

int main(void)
{
START:
    SHORT w = 12, h = 22;
    HANDLE hConsole = setupConsole(w * 2 + 30, h, L"Tetris");

    init();
    Scene *scene = newScene(w + 10, h);
    Object *map = newObject(0, 0, w, h);
    updateMap(map);
    Object *block = NULL;
    Object *nextBlock = newBlock(0, 0);
    FrameRate fr = newFrameRate(12);
    // 开场音效
    playSound2(7 * 3, 659, 100, 20,
               494, 100, 20,
               523, 100, 20,
               587, 100, 20,
               523, 100, 20,
               494, 100, 20,
               440, 100, 20);
    // system("cls");
    while (!isQuit)
    {
        frameStart(&fr);

        if (block)
        {
            if (block->status == 2)
            {
                goto START;
            }
            else if (block->status == 1)
            {
                freeObject(&block);
            }
        }

        if (!block)
        {
            block = nextBlock;
            block->x = map->w / 2 - block->w / 2;
            block->y = -block->h / 2;
            nextBlock = newBlock(w + 1, 1);
        }

        int k = getKb();
        if (k == 27)
            isQuit = true;
        else if (k == ' ')
        {
            isPaused = !isPaused;
        }
        else if (k == 'M')
        {
            scene->mode = (RenderMode)((scene->mode + 1) % RM_CNT);
            config.mode = scene->mode;
        }
        else if (k == 'A' || k == 0xE04B)
        {
            if (!isCollided(block, map, -1, 0))
            {
                block->x -= 1;
                playSound2(3, 800, 30, 0);
            }
        }
        else if (k == 'D' || k == 0xE04D)
        {
            if (!isCollided(block, map, 1, 0))
            {
                block->x += 1;
                playSound2(3, 800, 30, 0);
            }
        }
        else if (k == 'W' || k == 0xE048)
        {
            rotateBlock(block, map);
        }
        else if (k == 'S' || k == 0xE050)
        {
            if (!isCollided(block, map, 0, 1))
            {
                block->y += 1;
                playSound2(3, 800, 30, 0);
            }
        }
        else if (k == 'F')
        {
            while (!isCollided(block, map, 0, 1))
                block->y += 1;
        }
        else if (k == 'Q')
        {
            if (!isCollided(block, map, 0, -1))
                block->y -= 1;
        }

        if (!isPaused)
        {
            updateBlock(block, map);
        }

        updateMapToScene(scene, map);
        updateBlockToScene(scene, block, map);
        updateNextBlockToScene(scene, nextBlock);

        if (score > config.maxScore)
            config.maxScore = score;

        SHORT row = nextBlock->y + nextBlock->h + 1;
        SHORT col = w + 1;
        updateStrToScene(scene, col, 0, "下一个:");
        updateStrToScene(scene, col, row++, "得分: %6d", score);
        updateStrToScene(scene, col, row++, "最高分: %4d", config.maxScore);
        updateStrToScene(scene, col, row++, "输出模式: %d，按M切换", scene->mode);
        updateStrToScene(scene, col, row++, "移动: ASD|方向键");
        updateStrToScene(scene, col, row++, "旋转: W|↑键");
        updateStrToScene(scene, col, row++, "速降: F键");
        updateStrToScene(scene, col, row++, "退出: Esc键");
        updateStrToScene(scene, col, row++, "FPS: %.2f", 1000.0 / fr.elapsedMs);

        renderScene(scene);

        frameEnd(&fr);
    }

    freeObject(&map);
    freeScene(&scene);

    CloseHandle(hConsole);
    return 0;
}
