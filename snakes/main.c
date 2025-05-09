#include "../core/util.h"
#include "../core/console.h"
#include "../core/render.h"
#include "../core/music.h"
#include "map.h"
#include "food.h"
#include "snake.h"

#define CONFILE "config.dat"

typedef enum
{
    S_START,
    S_READY,
    S_PAUSED,
    S_OVER,
    S_RESTART,
    S_QUIT,
    S_WIN = 1 << 7,
} Status;

typedef struct
{
    RenderMode mode;
    int maxScore;
    bool mute;
} Config;
Config config = {.mode = RM_CH, .maxScore = 0, .mute = false};

FrameRate fr;
SHORT w = 20, h = 20;
SHORT messageW = 15;
Status status = S_START;

Scene *scene = NULL;
Object *map = NULL;
Snake **snakes = NULL;
SHORT snakeCount = 10;
SHORT aliveCount = 0;
Food *food = NULL;

void handleInputEvent(INPUT_RECORD *record, WORD vkCode, DWORD mState)
{
    // 显式忽略
    (void)record;
    (void)mState;

    if (vkCode == 'A' || vkCode == VK_LEFT)
    {
        if (status & S_READY && snakes[0]->dir != DIR_RIGHT)
        {
            snakes[0]->dir = DIR_LEFT;
            playSoundSeq(3, 300, 5, 0);
        }
    }
    else if (vkCode == 'D' || vkCode == VK_RIGHT)
    {
        if (status & S_READY && snakes[0]->dir != DIR_LEFT)
        {
            snakes[0]->dir = DIR_RIGHT;
            playSoundSeq(3, 300, 5, 0);
        }
    }
    else if (vkCode == 'W' || vkCode == VK_UP)
    {
        if (status & S_READY && snakes[0]->dir != DIR_DOWN)
        {
            snakes[0]->dir = DIR_UP;
            playSoundSeq(3, 300, 5, 0);
        }
    }
    else if (vkCode == 'S' || vkCode == VK_DOWN)
    {
        if (status & S_READY && snakes[0]->dir != DIR_UP)
        {
            snakes[0]->dir = DIR_DOWN;
            playSoundSeq(3, 300, 5, 0);
        }
    }
    else if (vkCode == 'M')
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
        {
            scene->mode |= RM_V;
        }
        config.mode = scene->mode;
    }
    else if (vkCode == 'P' || vkCode == VK_SPACE)
    {
        if (snakes[0]->alive)
        {
            if (status & S_READY)
            {
                status ^= S_READY;
                status |= S_PAUSED;
            }
            else
            {
                status ^= S_PAUSED;
                status |= S_READY;
            }
        }
        else
            status = S_RESTART;
    }
    else if (vkCode == 'K')
        config.mute = !config.mute;
    else if (vkCode == VK_ESCAPE)
        status = S_QUIT;
}

void setMessage(void)
{
    static SHORT row = 0;
    clearSceneP(scene, scene->camera.x + scene->camera.w - messageW, scene->camera.y, messageW, row - scene->camera.y);

    row = scene->camera.y;
    SHORT col = scene->camera.x + scene->camera.w - messageW;
    setSceneStr(scene, col, row++, Rgb(120, 120, 0), "得  分: %5d", snakes[0]->score);
    setSceneStr(scene, col, row++, Rgb(150, 100, 0), "最高分: %5d", config.maxScore);
    setSceneStr(scene, col, row++, Rgb(120, 120, 120), "玩家: %d/%d", aliveCount, snakeCount);
    if (status & S_WIN)
        setSceneStr(scene, col, row++, Rgb(200, 200, 0), "胜利😄");
    else if (!snakes[0]->alive)
        setSceneStr(scene, col, row++, Rgb(150, 50, 0), "失败😂");
    else
        setSceneStr(scene, col, row++, Rgb(0, 0, 0), "");
    setSceneStr(scene, col, row++, Rgb(0, 120, 120), "输出模式: %d，按M切换", scene->mode);
    setSceneStr(scene, col, row++, Rgb(120, 120, 120), "方向控制: WASD|方向键");
    if (snakes[0]->alive == false)
    {
        static SHORT count = 0;
        if (count++ > 10)
            count = 0;
        setSceneStr(scene, col, row++, Rgb(0, count > 5 ? 200 : 100, 0), "重新开始: P|空格键");
    }
    else
        setSceneStr(scene, col, row++, Rgb(100, 100, 100), "暂停|继续: P|空格键");
    setSceneStr(scene, col, row++, Rgb(100, 100, 100), "退出: Esc键");
    setSceneStr(scene, col, row++, Rgb(100, 100, 100), config.mute ? "打开音效: K键" : "关闭音效: K键");
    setSceneStr(scene, col, row++, Rgb(80, 80, 80), "帧率: %5.2fFps", 1000.0 / fr.elapsedMs);
}

void setCamera(void)
{
    Snake *snake = NULL;
    // 摄像机跟随存活者
    for (SHORT i = 0; i < snakeCount; i++)
    {
        if (snakes[i]->alive)
        {
            snake = snakes[i];
            break;
        }
    }
    if (snake == NULL)
        return;

    Node *node = snake->tail;
    while (node->next != NULL)
        node = node->next;

    scene->camera.x = node->x - (scene->camera.w - messageW) / 2;
    scene->camera.y = node->y - scene->camera.h / 2;

    // 限位
    if (scene->camera.x + scene->camera.w > scene->w)
        scene->camera.x = scene->w - scene->camera.w;
    if (scene->camera.y + scene->camera.h > scene->h)
        scene->camera.y = scene->h - scene->camera.h;
    if (scene->camera.x < 0)
        scene->camera.x = 0;
    if (scene->camera.y < 0)
        scene->camera.y = 0;
}

void quit(void)
{
    saveConfig(CONFILE, &config, sizeof(Config));
}

int main(void)
{
    loadConfig(CONFILE, &config, sizeof(Config));
    mute = &config.mute;
    setupConsole((w + messageW) * 2, h, L"贪吃蛇");
    setupClose(quit);

    scene = newScene(w + messageW + 10, h + 10);
    scene->camera.w = w + messageW;
    scene->camera.h = h;
    scene->mode = config.mode;
    map = newObject(0, 0, scene->w - messageW, scene->h);

START:
    status = S_READY;
    if (snakes)
    {
        for (SHORT i = 0; i < snakeCount; i++)
            freeSnake(&snakes[i]);
        free(snakes);
    }
    snakes = (Snake **)malloc(sizeof(Snake *) * snakeCount);
    for (SHORT i = 0; i < snakeCount; i++)
    {
        snakes[i] = newSnake(rand() % (w - 2) + 1, rand() % (h - 2) + 1);
        snakes[i]->speed = rand() % 12 + 4;
    }
    // 玩家
    snakes[0]->tail->x = w / 4;
    snakes[0]->tail->y = h / 2;
    snakes[0]->speed = 6;
    snakes[0]->npc = false;

    if (food)
        freeFood(&food);
    food = newFood(map->w * map->h / 30);

    // 开场音乐
    int data[] = {100, 50, 20, 200, 50, 20, 300, 50, 20, 400, 100, 20, 300, 50, 20, 200, 50, 20, 100, 50, 20};
    SoundParams soundParams = {.data = data, .count = ARRAY_LEN(data)};
    soundParams.callback = startSnake;
    soundParams.callbackContext = snakes[0];
    playSound(&soundParams);

    initMap(map);

    fr = newFrameRate(12);
    while (status != S_QUIT)
    {
        frameStart(&fr);

        getInput(handleInputEvent);
        if (status == S_RESTART)
            goto START;

        if (status & S_READY)
        {
            for (SHORT i = 0; i < snakeCount; i++)
            {
                updateSnake(snakes, snakeCount, i, food, map);
            }
            updateFood(food, snakes, snakeCount, map);
        }

        setSceneMap(scene, map);
        for (SHORT i = 0; i < snakeCount; i++)
            setSceneSnake(scene, snakes[i]);
        setSceneFood(scene, food);

        if (snakes[0]->score > config.maxScore)
        {
            config.maxScore = snakes[0]->score;
        }

        setCamera();
        setMessage();
        renderScene(scene);

        aliveCount = getAliveCount(snakes, snakeCount);
        if ((status & S_WIN) == 0 && aliveCount == 1 && snakes[0]->alive)
        {
            status |= S_WIN;
            // 胜利音乐
            playSoundSeq(12,
                         523, 200, 50, // C5 (Do)
                         587, 200, 50, // D5 (Re)
                         659, 200, 50, // E5 (Mi)
                         784, 400, 0);
        }

        frameEnd(&fr);
    }

    return 0;
}
