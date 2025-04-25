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

int main(void)
{
    init();
    SHORT w = 12, h = 22;
    HANDLE hConsole = setupConsole(w * 2 + 20, h, L"Tetris");

    Scene *scene = newScene(w, h);
    Object *map = newObject(0, 0, w, h);
    updateMap(map);
    Object *block = NULL;
    FrameRate fr = newFrameRate(12);
    while (!isQuit)
    {
        frameStart(&fr);

        if (!block || block->status == 1)
        {
            if (block)
                freeObject(&block);
            block = newBlock();
            block->x = map->w / 2 - block->w / 2;
            block->y = 0;
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
                block->x -= 1;
        }
        else if (k == 'D' || k == 0xE04D)
        {
            if (!isCollided(block, map, 1, 0))
                block->x += 1;
        }
        else if (k == 'W' || k == 0xE048)
        {
            rotateBlock(block, map);
        }
        else if (k == 'S' || k == 0xE050)
        {
            if (!isCollided(block, map, 0, 1))
                block->y += 1;
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

        renderScene(scene);

        {
            SHORT row = 0;
            SHORT col = scene->camera.w * 2 + 1;
            printStr(col, row++, 0x09, "最高分: %4d", config.maxScore);
            printStr(col, row++, 0x03, "输出模式: %d，按M切换", scene->mode);
            printStr(col, row++, 0x08, "方向控制: WASD|方向键");
            printStr(col, row++, 0x08, "退出: Esc键");
            printStr(col, row++, 0x08, "FPS: %.2f", 1000.0 / fr.elapsedMs);
        }

        frameEnd(&fr);
    }

    freeObject(&map);
    freeScene(&scene);

    CloseHandle(hConsole);
    return 0;
}
