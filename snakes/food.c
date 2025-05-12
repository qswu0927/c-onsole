#include "food.h"
#include <windows.h>
#include "../core/console.h"
#include "snake.h"

Food *newFood(SHORT maxLen)
{
    Food *food = (Food *)malloc(sizeof(Food));
    food->first = NULL;
    food->maxLen = maxLen;
    return food;
}

void updateFood(Food *food, Snake **snakes, SHORT snakeCount, Object *map)
{
    if (listLen(food->first) >= food->maxLen)
        return;
    SHORT x = rand() % (map->w - 2) + 1;
    SHORT y = rand() % (map->h - 2) + 1;
    // 食物不能出现在蛇身上
    for (SHORT i = 0; i < snakeCount; i++)
    {
        for (Node *node = snakes[i]->tail; node != NULL; node = node->next)
        {
            if (node->x == x && node->y == y)
                // 下一帧再生成
                return;
        }
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

wchar_t *getFoodCh(Scene *scene, SHORT data)
{
    if (scene->mode & RM_BG)
        return _wcsdup(L"  ");
    if (scene->mode & RM_CH)
    {
        switch (data)
        {
        case 0:
            return _wcsdup(L"鸟");
        case 1:
            return _wcsdup(L"蛙");
        case 2:
            return _wcsdup(L"鱼");
        default:
            return _wcsdup(L"鼠");
        }
    }
    switch (data)
    {
    case 0:
        return _wcsdup(L"🍎");
    case 1:
        return _wcsdup(L"🍖");
    case 2:
        return _wcsdup(L"🍉");
    default:
        return _wcsdup(L"🍄");
    }
}

Rgb getFoodRgb(Scene *scene, SHORT data)
{
    if (scene->mode & RM_BG)
        return Rgbb(40, 120, 0);
    if (scene->mode & RM_CH)
    {
        switch (data)
        {
        case 0:
            return Rgb(147, 112, 219);
        case 1:
            return Rgb(0, 128, 0);
        case 2:
            return Rgb(0, 191, 255);
        default:
            return Rgb(190, 190, 190);
        }
    }
    return Rgb(255, 165, 0);
}

void setSceneFood(Scene *scene, Food *food)
{
    for (Node *node = food->first; node != NULL; node = node->next)
    {
        scene->buffer[node->y][node->x].ch = getFoodCh(scene, node->data);
        scene->buffer[node->y][node->x].color.rgb = getFoodRgb(scene, node->data);
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

void freeFood(Food **foodPtr)
{
    if (!foodPtr || !*foodPtr)
        return;
    Food *food = *foodPtr;
    while (food->first != NULL)
    {
        Node *current = food->first;
        food->first = food->first->next;
        free(current);
    }
    free(food);
    *foodPtr = NULL;
}
