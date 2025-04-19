#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <stdbool.h>
#include <conio.h>
#include "core/util.h"
#include "core/window.h"
#include "core/render.h"
#include "core/music.h"
#include "map.h"

typedef struct
{
    // 为方便操作节点，链表从尾指向头
    Node *tail;
    Direction dir;
    bool alive;
    int score;
} Snake;

typedef struct
{
    // 支持同时存在多个食物，通过链表连接
    Node *first;
} Food;

wchar_t *getFoodCh(Scene *scene, SHORT data)
{
    if (scene->mode == RM_BG)
        return L"  ";
    return data == 0 ? L"🍎" : data == 1 ? L"鸡"
                           : data == 2   ? L"🍇"
                                         : L"🍉";
}

Food *newFood(void)
{
    Food *food = (Food *)malloc(sizeof(Food));
    food->first = NULL;
    return food;
}

void updateFood(Food *food, Snake *snake, Scene *scene)
{
    if (listLen(food->first) >= 50)
        return;
    SHORT x = rand() % (scene->w - 2) + 1;
    SHORT y = rand() % (scene->h - 2) + 1;
    // 食物不能出现在蛇身上
    for (Node *node = snake->tail; node != NULL; node = node->next)
    {
        if (node->x == x && node->y == y)
            // 下一帧再生成
            return;
    }
    // 食物不能出现在食物身上
    for (Node *node = food->first; node != NULL; node = node->next)
    {
        if (node->x == x && node->y == y)
            // 下一帧再生成
            return;
    }

    Node *node = (Node *)malloc(sizeof(Node));
    node->x = x;
    node->y = y;
    // 不同食物类型
    node->data = rand() % 4;
    node->next = food->first;
    food->first = node;
}

void updateFoodToScene(Scene *scene, Food *food)
{
    for (Node *node = food->first; node != NULL; node = node->next)
    {
        scene->buffer[node->y][node->x].ch = getFoodCh(scene, node->data);
        scene->buffer[node->y][node->x].color = scene->mode == RM_BG ? BACKGROUND_RED : 0;
    }
}

SHORT eatFood(Food *food, SHORT x, SHORT y)
{
    if (food->first == NULL)
        return -1;

    Node *current = food->first;
    Node *prev = NULL;

    while (current != NULL && !(current->x == x && current->y == y))
    {
        prev = current;
        current = current->next;
    }

    if (current == NULL)
        return -1;

    if (prev == NULL)
    {
        food->first = current->next;
    }
    else
    {
        prev->next = current->next;
    }

    SHORT data = current->data;
    free(current);

    return data;
}

void freeFood(Food *food)
{
    while (food->first != NULL)
    {
        Node *current = food->first;
        food->first = food->first->next;
        free(current);
    }
    free(food);
    food = NULL;
}

Snake *newSnake(SHORT x, SHORT y)
{
    Snake *snake = (Snake *)malloc(sizeof(Snake));
    Node *node = (Node *)malloc(sizeof(Node));
    node->x = x;
    node->y = y;
    node->next = NULL;
    snake->tail = node;
    snake->dir = DIR_NONE;
    snake->alive = true;
    snake->score = 0;
    return snake;
}

void startSnake(void *context)
{
    Snake *snake = (Snake *)context;
    snake->dir = DIR_RIGHT;
}

void updateSnake(Snake *snake, Food *food, Scene *scene)
{
    if (snake->dir == DIR_NONE)
        return;
    // 找到脖子节点
    Node *neckNode = snake->tail;
    while (neckNode->next != NULL && neckNode->next->next != NULL)
    {
        neckNode = neckNode->next;
    }
    // 找到头节点，考虑只有一个节点的情况
    Node *headNode = neckNode->next == NULL ? neckNode : neckNode->next;

    SHORT x = headNode->x;
    SHORT y = headNode->y;
    if (snake->dir == DIR_UP)
        y--;
    else if (snake->dir == DIR_DOWN)
        y++;
    else if (snake->dir == DIR_LEFT)
        x--;
    else if (snake->dir == DIR_RIGHT)
        x++;

    // 吃到食物
    SHORT eat = eatFood(food, x, y);
    if (eat >= 0)
    {
        snake->score++;
        // 播放音效
        Beep(600, 5);
        Node *newNode = (Node *)malloc(sizeof(Node));
        newNode->data = eat;
        // 新节点位置是原头部位置
        newNode->x = headNode->x;
        newNode->y = headNode->y;
        newNode->next = headNode;
        // 头部位置更新
        headNode->x = x;
        headNode->y = y;
        if (neckNode != headNode)
            neckNode->next = newNode;
        else
            snake->tail = newNode;
    }
    else
    {
        // 从尾到头遍历链表，更新蛇的位置
        Node *node = snake->tail;
        for (; node->next != NULL; node = node->next)
        {
            node->x = node->next->x;
            node->y = node->next->y;
            // 判断撞到自己
            if (snake->alive && node->x == x && node->y == y)
                snake->alive = false;
        }
        // 更新头节点
        node->x = x;
        node->y = y;

        // 判断撞墙
        if (x <= 0 || x >= scene->w - 1 || y <= 0 || y >= scene->h - 1)
            snake->alive = false;
    }
}

void updateSnakeToScene(Scene *scene, Snake *snake)
{
    for (Node *node = snake->tail; node != NULL; node = node->next)
    {
        scene->buffer[node->y][node->x].ch = node->next == NULL ? (snake->alive ? L"🤢" : L"😂") : getFoodCh(scene, node->data);
        scene->buffer[node->y][node->x].color = node->next == NULL ? (scene->mode == RM_BG ? BACKGROUND_RED | BACKGROUND_GREEN : FOREGROUND_GREEN) : BACKGROUND_GREEN;

        if (node->next == NULL)
        {
            // 摄像机跟随
            scene->camera.x = node->x - scene->camera.w / 2;
            scene->camera.y = node->y - scene->camera.h / 2;

            // 限位
            if (scene->camera.x < 0)
                scene->camera.x = 0;
            if (scene->camera.y < 0)
                scene->camera.y = 0;
            if (scene->camera.x + scene->camera.w > scene->w)
                scene->camera.x = scene->w - scene->camera.w;
            if (scene->camera.y + scene->camera.h > scene->h)
                scene->camera.y = scene->h - scene->camera.h;
        }
    }
}

void freeSnake(Snake *snake)
{
    while (snake->tail != NULL)
    {
        Node *current = snake->tail;
        snake->tail = snake->tail->next;
        free(current);
    }
    free(snake);
    snake = NULL;
}

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
    HANDLE hConsole = setupConsole(w, h, L"贪吃蛇");
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

    freeFood(food);
    freeSnake(snake);

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
