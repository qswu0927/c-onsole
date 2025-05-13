#ifndef SNAKE_H
#define SNAKE_H
#include <windows.h>
#include <stdbool.h>
#include "../core/util.h"
#include "../core/render.h"

#define SPEED_L 6
#define SPEED_H 15

struct Food;

typedef struct Snake
{
    // 为方便操作节点，链表从尾指向头
    Node *tail;
    // 当前方向
    Direction dir;
    // 设定方向
    Direction setDir;
    SHORT speed;
    SHORT tick;
    bool alive;
    int score;
    bool npc;
    bool focus;
} Snake;

Snake *newSnake(Object *map, bool npc);
void startSnake(void *context);
void updateSnake(Snake **snakes, SHORT snakeCount, SHORT index, struct Food *food, Object *map, Scene *scene);
void setSceneSnake(Scene *scene, Snake *snake);
SHORT getAliveCount(Snake **snakes, SHORT snakeCount);
void freeSnake(Snake **snakePtr);
void changeFocus(Snake **snakes, SHORT snakeCount, SHORT *focusOn);
void changeFocusNear(Snake **snakes, SHORT snakeCount, SHORT *focusOn);

#endif
