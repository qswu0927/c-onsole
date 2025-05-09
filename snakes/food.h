#ifndef FOOD_H
#define FOOD_H
#include "../core/util.h"
#include "../core/render.h"

struct Snake;

typedef struct Food
{
    // 支持同时存在多个食物，通过链表连接
    Node *first;
    SHORT maxLen;
} Food;

wchar_t *getFoodCh(Scene *scene, SHORT data);
Rgb getFoodRgb(Scene *scene, SHORT data);
Food *newFood(SHORT maxLen);
void updateFood(Food *food, struct Snake **snakes, SHORT snakeCount, Object *map);
void setSceneFood(Scene *scene, Food *food);
SHORT eatFood(Food *food, SHORT x, SHORT y);
void freeFood(Food **foodPtr);

#endif
