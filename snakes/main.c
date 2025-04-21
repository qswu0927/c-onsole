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
    Snake *snake = newSnake(w / 4, h / 2);
    Food *food = newFood();

    // 开场音乐
    SoundParams soundParams = {.frequencies = (int[]){100, 200, 300, 400, 300, 200, 100}, .durations = (int[]){50, 50, 50, 100, 50, 50, 50}, .count = 7, .pause = 20};
    soundParams.callback = startSnake;
    soundParams.callbackContext = snake;
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
            if (snake->dir != DIR_RIGHT && !isPaused)
                snake->dir = DIR_LEFT;
        }
        else if (k == 'D' || k == 0xE04D)
        {
            if (snake->dir != DIR_LEFT && !isPaused)
                snake->dir = DIR_RIGHT;
        }
        else if (k == 'W' || k == 0xE048)
        {
            if (snake->dir != DIR_DOWN && !isPaused)
                snake->dir = DIR_UP;
        }
        else if (k == 'S' || k == 0xE050)
        {
            if (snake->dir != DIR_UP && !isPaused)
                snake->dir = DIR_DOWN;
        }

        if (!isPaused)
        {
            updateSnake(snake, food, scene);
            updateFood(food, snake, scene);
        }

        updateMapToScene(scene, map);
        updateSnakeToScene(scene, snake);
        updateFoodToScene(scene, food);

        if (!snake->alive)
        {
            Beep(350, 100);
            break;
        }

        renderScene(scene);

        {
            SHORT row = 0;
            SHORT col = scene->camera.w * 2 + 1;
            printStr(col, row++, 0x0A, "得分: %6d", snake->score);
            printStr(col, row++, 0x09, "最高分: %4d", config.maxScore);
            printStr(col, row++, 0x80, "输出模式: %d，按M切换", scene->mode);
            printStr(col, row++, 0x08, "方向控制: WASD|方向键");
            printStr(col, row++, 0x08, "暂停|继续|重新开始: 空格键");
            printStr(col, row++, 0x08, "退出: Esc键");
            printStr(col, row++, 0x08, "FPS: %.2f", 1000.0 / fr.elapsedMs);

            if (snake->score > config.maxScore)
            {
                config.maxScore = snake->score;
            }
        }

        frameEnd(&fr);
    }

    freeFood(&food);
    freeSnake(&snake);

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
