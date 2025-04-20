#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <stdbool.h>
#include <conio.h>
#include "core/util.h"
#include "core/console.h"
#include "core/render.h"
#include "core/music.h"
#include "map.h"
#include "food.h"
#include "snake.h"

void keyboardControl(Scene *scene, Snake *snake)
{
    if (!_kbhit())
        return;
    switch (toupper(_getch()))
    {
    case 0xE0:
    case 0:
        switch (_getch())
        {
        case 72:
            if (snake->dir != DIR_DOWN)
                snake->dir = DIR_UP;
            break;
        case 80:
            if (snake->dir != DIR_UP)
                snake->dir = DIR_DOWN;
            break;
        case 75:
            if (snake->dir != DIR_RIGHT)
                snake->dir = DIR_LEFT;
            break;
        case 77:
            if (snake->dir != DIR_LEFT)
                snake->dir = DIR_RIGHT;
            break;
        }
        break;
    case 'A':
        if (snake->dir != DIR_RIGHT)
            snake->dir = DIR_LEFT;
        break;
    case 'D':
        if (snake->dir != DIR_LEFT)
            snake->dir = DIR_RIGHT;
        break;
    case 'W':
        if (snake->dir != DIR_DOWN)
            snake->dir = DIR_UP;
        break;
    case 'S':
        if (snake->dir != DIR_UP)
            snake->dir = DIR_DOWN;
        break;
    case 'M':
        scene->mode = (RenderMode)((scene->mode + 1) % RM_CNT);
        break;
    }
}

int main(void)
{
    SHORT w = 44, h = 33;
    HANDLE hConsole = setupConsole(60, 20, L"贪吃蛇");
    srand((unsigned int)time(NULL));

    Scene *scene = newScene(w, h);
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
    while (1)
    {
        frameStart(&fr);

        keyboardControl(scene, snake);

        updateSnake(snake, food, scene);
        updateFood(food, snake, scene);

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
            printStr(scene->camera.w * 2, row++, FOREGROUND_BLUE, "得分: %3d", snake->score);
            printStr(scene->camera.w * 2, row++, FOREGROUND_BLUE, "输出模式: %d，按M切换", scene->mode);
            printStr(scene->camera.w * 2, row++, FOREGROUND_BLUE, "FPS: %.2f", 1000.0 / fr.elapsedMs);
            if (!snake->alive)
            {
                printStr(scene->camera.w * 2, row++, FOREGROUND_RED, "按空格重新开始");
                printStr(scene->camera.w * 2, row++, FOREGROUND_RED, "按Esc退出");
            }
        }

        frameEnd(&fr);
    }

    freeFood(&food);
    freeSnake(&snake);

    while (1)
    {
        if (_kbhit())
        {
            int key = _getch();
            if (key == ' ')
                goto BEGIN;
            // Esc退出
            if (key == 27)
                break;
        }
        Sleep(100);
    }

    freeObject(&map);
    freeScene(&scene);

    CloseHandle(hConsole);
    return 0;
}
