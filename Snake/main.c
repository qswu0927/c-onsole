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
    struct Node *prev;
} Node;

typedef struct
{
    Node *head;
    Node *tail;
    Direction dir;
} Snake;

typedef struct
{
    Node *last;
} Food;

int score = 0;

bool isAlive(Snake *snake, Scene *scene)
{
    if (snake->head->x <= 0 || snake->head->x >= scene->w - 1 || snake->head->y <= 0 || snake->head->y >= scene->h - 1)
        return false;

    for (Node *node = snake->tail; node != snake->head; node = node->prev)
    {
        if (node->x == snake->head->x && node->y == snake->head->y)
            return false;
    }
    return true;
}

void updateWall(Object *obj)
{
    for (int i = 0; i < obj->h; i++)
    {
        for (int j = 0; j < obj->w; j++)
        {
            if (i == 0 || i == obj->h - 1 || j == 0 || j == obj->w - 1)
                obj->buffer[i][j] = BACKGROUND_BLUE;
            else
                obj->buffer[i][j] = 0;
        }
    }
}

SHORT getFoodLength(Food *food)
{
    SHORT length = 0;
    for (Node *node = food->last; node != NULL; node = node->prev)
        length++;
    return length;
}

Food *newFood()
{
    Food *food = (Food *)malloc(sizeof(Food));
    food->last = NULL;
    return food;
}

void updateFood(Food *food, Snake *snake, Scene *scene)
{
    if (getFoodLength(food) > 100)
        return;
    SHORT x = rand() % scene->w;
    SHORT y = rand() % scene->h;
    if (scene->buffer[y][x] != 0)
    {
        // 下一帧再生成
        return;
    }
    Node *node = (Node *)malloc(sizeof(Node));
    node->x = x;
    node->y = y;
    node->prev = food->last;
    food->last = node;
}

void addFoodToScene(Scene *scene, Food *food)
{
    for (Node *node = food->last; node != NULL; node = node->prev)
    {
        if (node->y >= 0 && node->y < scene->h && node->x >= 0 && node->x < scene->w)
            scene->buffer[node->y][node->x] = BACKGROUND_RED;
    }
}

bool eatFood(Food *food, Node *target)
{
    if (food->last == NULL)
        return false;

    Node *current = food->last;
    Node *prev = NULL;

    while (current != NULL && !(current->x == target->x && current->y == target->y))
    {
        prev = current;
        current = current->prev;
    }

    if (current == NULL)
        return false;

    if (prev == NULL)
    {
        food->last = current->prev;
    }
    else
    {
        prev->prev = current->prev;
    }
    free(current);
    score++;
    return true;
}

void freeFood(Food *food)
{
    while (food->last != NULL)
    {
        Node *current = food->last;
        food->last = food->last->prev;
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
    node->prev = NULL;
    snake->head = node;
    snake->tail = node;
    snake->dir = RIGHT;
    return snake;
}

void updateSnake(Snake *snake, Food *food, Scene *scene)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->x = snake->head->x;
    newNode->y = snake->head->y;
    newNode->prev = NULL;
    if (snake->dir == UP)
        newNode->y--;
    else if (snake->dir == DOWN)
        newNode->y++;
    else if (snake->dir == LEFT)
        newNode->x--;
    else if (snake->dir == RIGHT)
        newNode->x++;

    // 吃到食物
    bool eat = eatFood(food, newNode);

    snake->head->prev = newNode;
    snake->head = newNode;

    // 删除尾节点
    if (!eat)
    {
        Node *tail = snake->tail;
        snake->tail = tail->prev;
        free(tail);
        tail = NULL;
    }
}

void addSnakeToScene(Scene *scene, Snake *snake)
{
    for (Node *node = snake->tail; node != NULL; node = node->prev)
    {
        if (node->y >= 0 && node->y < scene->h && node->x >= 0 && node->x < scene->w)
            scene->buffer[node->y][node->x] = node->prev == NULL ? (char)(BACKGROUND_GREEN | BACKGROUND_RED) : BACKGROUND_GREEN;
    }
}

void freeSnake(Snake *snake)
{
    while (snake->tail != NULL)
    {
        Node *current = snake->tail;
        snake->tail = snake->tail->prev;
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

    FrameRate fr = newFrameRate(10);
    while (1)
    {
        frameStart(&fr);

        keyboardControl(snake);

        updateWall(wall);
        updateSnake(snake, food, scene);
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

        if (!isAlive(snake, scene))
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