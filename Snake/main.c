#include <stdio.h>
#include <windows.h>
#include <stdbool.h>
#include <conio.h>
#include "core/window.h"
#include "core/render.h"

typedef enum
{
    NONE,
    UP,
    DOWN,
    LEFT,
    RIGHT
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
                wall->buffer[i][j].ch = L"🚧";
            else
                wall->buffer[i][j].ch = L"  ";
        }
    }
}

wchar_t *getFoodCh(SHORT data)
{
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
    if (listLength(food->first) > 5)
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
        scene->buffer[node->y][node->x].ch = getFoodCh(node->data);
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
    snake->dir = RIGHT;
    return snake;
}

bool updateSnake(Snake *snake, Food *food, Scene *scene)
{
    bool alive = true;

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
    if (snake->dir == UP)
        y--;
    else if (snake->dir == DOWN)
        y++;
    else if (snake->dir == LEFT)
        x--;
    else if (snake->dir == RIGHT)
        x++;

    // 吃到食物
    SHORT eat = eatFood(food, x, y);
    if (eat >= 0)
    {
        score++;
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
            if (alive && node->x == x && node->y == y)
                alive = false;
        }
        // 更新头节点
        node->x = x;
        node->y = y;

        // 判断撞墙
        if (x <= 0 || x >= scene->w - 1 || y <= 0 || y >= scene->h - 1)
            alive = false;
    }
    return alive;
}

void addSnakeToScene(Scene *scene, Snake *snake)
{
    for (Node *node = snake->tail; node != NULL; node = node->next)
    {
        scene->buffer[node->y][node->x].ch = node->next == NULL ? L"🤢" : getFoodCh(node->data);
        scene->buffer[node->y][node->x].color = node->next == NULL ? (char)(FOREGROUND_GREEN | FOREGROUND_RED) : BACKGROUND_GREEN;
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

Direction getInput()
{
    if (_kbhit())
    {
        int ch = _getch();
        if (ch == 0xE0 || ch == 0)
        {
            switch (_getch())
            {
            case 72:
                return UP;
            case 80:
                return DOWN;
            case 75:
                return LEFT;
            case 77:
                return RIGHT;
            }
        }
        else if (ch == 'a' || ch == 'A')
            return LEFT;
        else if (ch == 'd' || ch == 'D')
            return RIGHT;
        else if (ch == 'w' || ch == 'W')
            return UP;
        else if (ch == 's' || ch == 'S')
            return DOWN;
    }
    return NONE;
}

void keyboardControl(Snake *snake)
{
    Direction dir = getInput();

    switch (dir)
    {
    case UP:
        if (snake->dir != DOWN)
            snake->dir = UP;
        break;
    case DOWN:
        if (snake->dir != UP)
            snake->dir = DOWN;
        break;
    case LEFT:
        if (snake->dir != RIGHT)
            snake->dir = LEFT;
        break;
    case RIGHT:
        if (snake->dir != LEFT)
            snake->dir = RIGHT;
        break;
    case NONE:
        break;
    }
}

int main()
{
start:
    SHORT w = 20, h = 20;
    HANDLE hConsole = setupConsole(w, h, L"贪吃蛇");

    Scene *scene = newScene(w, h);
    Object *wall = newObject(0, 0, w, h);
    Snake *snake = newSnake(w / 3, h / 2);
    Food *food = newFood();

    initWall(wall);

    FrameRate fr = newFrameRate(8);
    while (1)
    {
        frameStart(&fr);

        keyboardControl(snake);

        bool alive = updateSnake(snake, food, scene);
        updateFood(food, snake, scene);

        addToScene(scene, wall);
        addSnakeToScene(scene, snake);
        addFoodToScene(scene, food);

        renderScene(scene);

        char buffer[100];
        snprintf(buffer, sizeof(buffer), "%s: %d", "得分", score);
        printString(w * 2, 0, FOREGROUND_GREEN | FOREGROUND_BLUE, buffer);
        snprintf(buffer, sizeof(buffer), "%s: %.2f", "FPS", 1000.0 / fr.elapsedMs);
        printString(w * 2, 2, FOREGROUND_GREEN | FOREGROUND_BLUE, buffer);
        snprintf(buffer, sizeof(buffer), "%s: %d", "FOOD", listLength(food->first));
        printString(w * 2, 3, FOREGROUND_GREEN | FOREGROUND_BLUE, buffer);
        printString(w * 2, 4, FOREGROUND_GREEN | FOREGROUND_BLUE, listToString(food->first));
        snprintf(buffer, sizeof(buffer), "%s: %d", "SNAKE", listLength(snake->tail));
        printString(w * 2, 5, FOREGROUND_GREEN | FOREGROUND_BLUE, buffer);

        if (!alive)
            break;

        frameEnd(&fr);
    }

    freeFood(food);
    freeSnake(snake);
    freeObject(wall);
    freeScene(scene);

    while (1)
    {
        printString(w * 2, h / 2, BACKGROUND_RED | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, "按【空格】键重新开始");
        if (_kbhit() && _getch() == ' ')
        {
            goto start;
        }
    }

    CloseHandle(hConsole);

    getchar();
    return 0;
}