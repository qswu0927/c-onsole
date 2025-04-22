#include <time.h>
#include <signal.h>
#include "../core/util.h"
#include "../core/console.h"
#include "../core/render.h"
#include "../core/music.h"
#include "map.h"
#include "food.h"
#include "snake.h"

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
    SHORT w = 44, h = 33;
    HANDLE hConsole = setupConsole(70, 20, L"贪吃蛇");

    Scene *scene = newScene(w, h);
    scene->mode = config.mode;
    scene->camera.w = 20;
    scene->camera.h = 20;
    Object *map = newObject(0, 0, w, h);

BEGIN:
    Snake **snakes;
    u_char snakeCount = 6;
    snakes = (Snake **)malloc(sizeof(Snake *) * snakeCount);
    for (int i = 0; i < snakeCount; i++)
    {
        snakes[i] = newSnake(rand() % (w - 2) + 1, rand() % (h - 2) + 1);
    }
    // 玩家
    snakes[0]->tail->x = w / 4;
    snakes[0]->tail->y = h / 2;
    snakes[0]->npc = false;

    Food *food = newFood();

    // 开场音乐
    SoundParams soundParams = {.frequencies = (int[]){100, 200, 300, 400, 300, 200, 100}, .durations = (int[]){50, 50, 50, 100, 50, 50, 50}, .count = 7, .pause = 20};
    soundParams.callback = startSnake;
    soundParams.callbackContext = snakes[0];
    playSound(&soundParams);

    updateMap(map);

    FrameRate fr = newFrameRate(6);
    while (!isQuit)
    {
        frameStart(&fr);

        int k = getKb();
        if (k == 27)
            isQuit = true;
        else if (k == ' ')
            isPaused = !isPaused;
        else if (k == 'M')
        {
            scene->mode = (RenderMode)((scene->mode + 1) % RM_CNT);
            config.mode = scene->mode;
        }
        else if (k == 'A' || k == 0xE04B)
        {
            if (snakes[0]->dir != DIR_RIGHT && !isPaused)
                snakes[0]->dir = DIR_LEFT;
        }
        else if (k == 'D' || k == 0xE04D)
        {
            if (snakes[0]->dir != DIR_LEFT && !isPaused)
                snakes[0]->dir = DIR_RIGHT;
        }
        else if (k == 'W' || k == 0xE048)
        {
            if (snakes[0]->dir != DIR_DOWN && !isPaused)
                snakes[0]->dir = DIR_UP;
        }
        else if (k == 'S' || k == 0xE050)
        {
            if (snakes[0]->dir != DIR_UP && !isPaused)
                snakes[0]->dir = DIR_DOWN;
        }

        if (!isPaused)
        {
            for (int i = 0; i < snakeCount; i++)
            {
                updateSnake(snakes, snakeCount, i, food, map);
            }
            updateFood(food, snakes, snakeCount, scene);
        }

        updateMapToScene(scene, map);
        for (int i = 0; i < snakeCount; i++)
        {
            updateSnakeToScene(scene, snakes[i]);
        }
        updateFoodToScene(scene, food);

        if (snakes[0]->score > config.maxScore)
        {
            config.maxScore = snakes[0]->score;
        }

        if (!snakes[0]->alive)
        {
            Beep(350, 100);
            break;
        }

        renderScene(scene);

        {
            SHORT row = 0;
            SHORT col = scene->camera.w * 2 + 1;
            printStr(col, row++, 0x0A, "得分: %6d", snakes[0]->score);
            printStr(col, row++, 0x09, "最高分: %4d", config.maxScore);
            printStr(col, row++, 0x80, "输出模式: %d，按M切换", scene->mode);
            printStr(col, row++, 0x08, "方向控制: WASD|方向键");
            printStr(col, row++, 0x08, "暂停|继续|重新开始: 空格键");
            printStr(col, row++, 0x08, "退出: Esc键");
            printStr(col, row++, 0x08, "FPS: %.2f", 1000.0 / fr.elapsedMs);
        }

        frameEnd(&fr);
    }

    freeFood(&food);
    for (int i = 0; i < snakeCount; i++)
    {
        freeSnake(&snakes[i]);
    }
    free(snakes);

    while (!isQuit)
    {
        int k = getKb();
        if (k == ' ')
            goto BEGIN;
        Sleep(100);
    }

    freeObject(&map);
    freeScene(&scene);

    CloseHandle(hConsole);
    return 0;
}
