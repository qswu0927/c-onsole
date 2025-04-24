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

void updateFood(Food *food, Snake **snakes, u_char snakeCount, Object *map)
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
    if (scene->mode == RM_BG)
        return L"  ";
    if (scene->mode == RM_CH)
    {
        switch (data)
        {
        case 0:
            return L"鸟";
        case 1:
            return L"蛙";
        case 2:
            return L"鱼";
        default:
            return L"鼠";
        }
    }
    switch (data)
    {
    case 0:
        return L"🍎";
    case 1:
        return L"🍖";
    case 2:
        return L"🍉";
    default:
        return L"🍄";
    }
}

RGBColor getFoodRgb(Scene *scene, SHORT data)
{
    if (scene->mode == RM_BG)
    {
        switch (data)
        {
        case 0:
            return (RGBColor){0, 0, 0, 255, 165, 0};
        case 1:
            return (RGBColor){0, 0, 0, 0, 128, 0};
        case 2:
            return (RGBColor){0, 0, 0, 0, 191, 255};
        default:
            return (RGBColor){0, 0, 0, 211, 211, 211};
        }
    }
    if (scene->mode == RM_CH)
    {
        switch (data)
        {
        case 0:
            return (RGBColor){147, 112, 219, 0, 0, 0};
        case 1:
            return (RGBColor){0, 128, 0, 0, 0, 0};
        case 2:
            return (RGBColor){0, 191, 255, 0, 0, 0};
        default:
            return (RGBColor){190, 190, 190, 0, 0, 0};
        }
    }
    return (RGBColor){255, 255, 255, 0, 0, 0};
}

void updateFoodToScene(Scene *scene, Food *food)
{
    for (Node *node = food->first; node != NULL; node = node->next)
    {
        scene->buffer[node->y][node->x].ch = getFoodCh(scene, node->data);
        if (vtSupported)
        {
            scene->buffer[node->y][node->x].color.rgb = getFoodRgb(scene, node->data);
        }
        else
        {
            scene->buffer[node->y][node->x].color.attr = scene->mode == RM_BG ? BACKGROUND_RED : 0;
        }
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
