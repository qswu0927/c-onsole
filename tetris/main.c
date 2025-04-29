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
    setupConsole((w + 12) * 2, h, L"Tetris");

START:
    Scene *scene = newScene(w + 12, h);
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

        {
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
        }

        if (!isPaused)
        {
            updateBlock(block, map);
        }

        setSceneMap(scene, map);
        setSceneBlock(scene, block, map);
        setSceneNextBlock(scene, nextBlock);

        if (score > config.maxScore)
            config.maxScore = score;

        SHORT row = nextBlock->y + nextBlock->h + 1;
        SHORT col = w + 1;
        const Rgb rgb = {120, 120, 120, 0, 0, 0};
        setSceneStr(scene, col, 0, rgb, "下一个:");
        setSceneStr(scene, col, row++, rgb, "得分: %6d", score);
        setSceneStr(scene, col, row++, rgb, "最高分: %4d", config.maxScore);
        setSceneStr(scene, col, row++, rgb, "输出模式: %d，按M切换", scene->mode);
        setSceneStr(scene, col, row++, rgb, "移动: ASD|方向键");
        setSceneStr(scene, col, row++, rgb, "旋转: W|↑键");
        setSceneStr(scene, col, row++, rgb, "速降: F键");
        setSceneStr(scene, col, row++, rgb, "退出: Esc键");
        setSceneStr(scene, col, row++, rgb, "帧率: %5.2fFps", 1000.0 / fr.elapsedMs);

        renderScene(scene);

        frameEnd(&fr);
    }

    freeObject(&map);
    freeScene(&scene);

    CloseHandle(hConsole);
    return 0;
}
