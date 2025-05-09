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
    SHORT speed;
    SHORT tick;
    bool alive;
    int score;
    bool npc;
} Snake;

Snake *newSnake(SHORT x, SHORT y);
void startSnake(void *context);
void updateSnake(Snake **snakes, SHORT snakeCount, SHORT index, struct Food *food, Object *map);
void setSceneSnake(Scene *scene, Snake *snake);
SHORT getAliveCount(Snake **snakes, SHORT snakeCount);
void freeSnake(Snake **snakePtr);

#endif
