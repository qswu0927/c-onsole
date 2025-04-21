#ifndef SNAKE_H
#define SNAKE_H
#include <windows.h>
#include <stdbool.h>
#include "../core/util.h"
#include "../core/render.h"

struct Food;

typedef struct Snake
{
    // 为方便操作节点，链表从尾指向头
    Node *tail;
    Direction dir;
    bool alive;
    int score;
} Snake;

Snake *newSnake(SHORT x, SHORT y);
void startSnake(void *context);
void updateSnake(Snake *snake, struct Food *food, Scene *scene);
void updateSnakeToScene(Scene *scene, Snake *snake);
void freeSnake(Snake **snakePtr);

#endif
