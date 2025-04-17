#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <stdbool.h>
#include <conio.h>
#include "core/window.h"
#include "core/render.h"
#include "core/music.h"

/**
 * 前进方向
 */
typedef enum
{
    DIR_NONE,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

typedef struct Node
{
    SHORT x, y;
    SHORT data;
    struct Node *next;
} Node;

typedef struct
{
    // 为方便操作节点，链表从尾指向头
    Node *tail;
    // 前进方向
    Direction dir;
    // 存活
    bool alive;
} Snake;

typedef struct
{
    // 支持同时存在多个食物，通过链表连接
    Node *first;
} Food;

int score = 0;

char *listToString(Node *head)
{
    if (head == NULL)
    {
        char *str = malloc(1);
        str[0] = '\0';
        return str;
    }

    // 第一次遍历计算所需空间
    int length = 0;
    Node *current = head;
    while (current != NULL)
    {
        length += snprintf(NULL, 0, "%d", current->data) + 2; // 数字长度 + "->"
        current = current->next;
    }

    // 分配内存
    char *str = malloc(length);
    if (str == NULL)
        return NULL;

    // 第二次遍历构建字符串
    current = head;
    int pos = 0;
    while (current != NULL)
    {
        pos += sprintf(str + pos, "%d", current->data);
        if (current->next != NULL)
        {
            pos += sprintf(str + pos, "->");
        }
        current = current->next;
    }

    return str;
}

int listLength(Node *head)
{
    int length = 0;
    Node *current = head;
    while (current != NULL)
    {
        length++;
        current = current->next;
    }
    return length;
}

void initWall(Object *wall)
{
    for (int i = 0; i < wall->h; i++)
    {
        for (int j = 0; j < wall->w; j++)
        {
            if (i == 0 || i == wall->h - 1 || j == 0 || j == wall->w - 1)
                wall->data[i][j] = 1;
            else
                wall->data[i][j] = 0;
        }
    }
}

void addWallToScene(Scene *scene, Object *obj)
{
    for (int i = 0; i < obj->h; i++)
    {
        for (int j = 0; j < obj->w; j++)
        {
            if (i + obj->y >= 0 && i + obj->y < scene->h && j + obj->x >= 0 && j + obj->x < scene->w)
            {
                scene->buffer[i + obj->y][j + obj->x].ch = obj->data[i][j] == 1 ? (scene->mode == RM_EM ? L"🚧" : scene->mode == RM_CH ? L"墙"
                                                                                                                                       : L"  ")
                                                                                : L"  ";

                scene->buffer[i + obj->y][j + obj->x].color = obj->data[i][j] == 1 ? (scene->mode == RM_BG ? BACKGROUND_BLUE : 0x01) : 0;
            }
        }
    }
}

wchar_t *getFoodCh(Scene *scene, SHORT data)
{
    if (scene->mode == RM_BG)
        return L"  ";
    return data == 0 ? L"🍎" : data == 1 ? L"鸡"
                           : data == 2   ? L"🍇"
                                         : L"🍉";
}

Food *newFood()
{
    Food *food = (Food *)malloc(sizeof(Food));
    food->first = NULL;
    return food;
}

void updateFood(Food *food, Snake *snake, Scene *scene)
{
    if (listLength(food->first) >= 50)
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

void addFoodToScene(Scene *scene, Food *food)
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
        score++;
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

void addSnakeToScene(Scene *scene, Snake *snake)
{
    for (Node *node = snake->tail; node != NULL; node = node->next)
    {
        scene->buffer[node->y][node->x].ch = node->next == NULL ? (snake->alive ? L"🤢" : L"😂") : getFoodCh(scene, node->data);
        scene->buffer[node->y][node->x].color = node->next == NULL ? (scene->mode == RM_BG ? BACKGROUND_RED | BACKGROUND_GREEN : FOREGROUND_GREEN) : BACKGROUND_GREEN;
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

int main()
{
start:
    score = 0;
    SHORT w = 20, h = 20;
    HANDLE hConsole = setupConsole(w, h, L"贪吃蛇");
    // 使用当前时间作为随机数种子
    srand((unsigned int)time(NULL));

    Scene *scene = newScene(w, h);
    Object *wall = newObject(0, 0, w, h);
    Snake *snake = newSnake(w / 4, h / 2);
    Food *food = newFood();

    // 播放开始音乐
    SoundParams soundParams = {.frequencies = (int[]){100, 200, 300, 400, 300, 200, 100}, .durations = (int[]){50, 50, 50, 100, 50, 50, 50}, .count = 7, .pause = 20};
    // 音乐结束后开始运动
    soundParams.callback = startSnake;
    soundParams.callbackContext = snake;
    playSound(&soundParams);

    initWall(wall);

    FrameRate fr = newFrameRate(6);
    while (1)
    {
        frameStart(&fr);

        keyboardControl(scene, snake);

        updateSnake(snake, food, scene);
        updateFood(food, snake, scene);

        addWallToScene(scene, wall);
        addSnakeToScene(scene, snake);
        addFoodToScene(scene, food);

        renderScene(scene);

        char buffer[100];
        snprintf(buffer, sizeof(buffer), "%s: %d", "得分", score);
        printString(w * 2, 0, FOREGROUND_GREEN | FOREGROUND_BLUE, buffer);
        snprintf(buffer, sizeof(buffer), "%s: %d", "模式", scene->mode);
        printString(w * 2, 1, FOREGROUND_GREEN | FOREGROUND_BLUE, buffer);
        snprintf(buffer, sizeof(buffer), "%s: %.2f", "FPS", 1000.0 / fr.elapsedMs);
        printString(w * 2, 2, FOREGROUND_GREEN | FOREGROUND_BLUE, buffer);
        snprintf(buffer, sizeof(buffer), "%s: %d", "FOOD", listLength(food->first));
        printString(w * 2, 3, 0x03, buffer);
        // printString(w * 2, 4, FOREGROUND_GREEN | FOREGROUND_BLUE, listToString(food->first));
        snprintf(buffer, sizeof(buffer), "%s: %d", "SNAKE", listLength(snake->tail));
        printString(w * 2, 5, FOREGROUND_GREEN | FOREGROUND_BLUE, buffer);

        if (!snake->alive)
        {
            Beep(350, 100);
            break;
        }

        frameEnd(&fr);
    }

    freeFood(food);
    freeSnake(snake);
    freeObject(&wall);
    freeScene(&scene);

    while (1)
    {
        printString(w * 2, h / 2, BACKGROUND_RED | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, "按【空格】键重新开始");
        if (_kbhit() && _getch() == ' ')
        {
            goto start;
        }
        Sleep(100);
    }

    CloseHandle(hConsole);

    getchar();
    return 0;
}