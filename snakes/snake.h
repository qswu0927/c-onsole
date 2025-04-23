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
    bool npc;
} Snake;

Snake *newSnake(SHORT x, SHORT y);
void startSnake(void *context);
void updateSnake(Snake **snakes, u_char snakeCount, u_char index, struct Food *food, Object *map);
void updateSnakeToScene(Scene *scene, Snake *snake);
u_char getAliveCount(Snake **snakes, u_char snakeCount);
void freeSnake(Snake **snakePtr);

#endif
