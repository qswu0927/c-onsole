#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include "core/window.h"
#include "core/render.h"

typedef enum
{
    UP,
    DOWN,
    LEFT,
    RIGHT
} Direction;

typedef struct Node Node;
struct Node
{
    SHORT x, y;
    struct Node *prev;
};

typedef struct
{
    Node *head;
    Node *tail;
    Direction dir;
} Snake;

void updateWall(Object *obj)
{
    for (int i = 0; i < obj->h; i++)
    {
        for (int j = 0; j < obj->w; j++)
        {
            if (i == 0 || i == obj->h - 1 || j == 0 || j == obj->w - 1)
                obj->buffer[i][j] = '#';
            else
                obj->buffer[i][j] = ' ';
        }
    }
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

void updateSnake(Snake *snake)
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

    snake->head->prev = newNode;
    snake->head = newNode;
    // 删除尾节点
    Node *tail = snake->tail;
    snake->tail = tail->prev;
    free(tail);
    tail = NULL;
}

void addSnakeToScene(Scene *scene, Snake *snake)
{
    for (Node *node = snake->tail; node != NULL; node = node->prev)
    {
        if (node->y >= 0 && node->y < scene->h && node->x >= 0 && node->x < scene->w)
            scene->buffer[node->y][node->x] = 1;
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

int main()
{
    SHORT w = 20, h = 20;
    HANDLE hConsole = setupConsole(w, h, L"贪吃蛇");

    Scene *scene = newScene(w, h);
    Object *wall = newObject(0, 0, w, h);
    Snake *snake = newSnake(w / 2, h / 2);

    while (1)
    {
        if (_kbhit())
        {
            switch (_getch())
            {
            case 'w':
            case 'W':
                snake->dir = UP;
                break;
            case 's':
            case 'S':
                snake->dir = DOWN;
                break;
            case 'a':
            case 'A':
                snake->dir = LEFT;
                break;
            case 'd':
            case 'D':
                snake->dir = RIGHT;
                break;
            }
        }
        updateWall(wall);
        addToScene(scene, wall);
        updateSnake(snake);
        addSnakeToScene(scene, snake);
        renderScene(scene);
        Sleep(50);
    }

    freeSnake(snake);
    freeObject(wall);
    freeScene(scene);
    CloseHandle(hConsole);
    return 0;
}