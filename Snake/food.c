#include "food.h"
#include <windows.h>
#include "snake.h"

wchar_t *getFoodCh(Scene *scene, SHORT data)
{
    if (scene->mode == RM_BG)
        return L"  ";
    return data == 0 ? L"🍎" : data == 1 ? L"鸡"
                           : data == 2   ? L"🍇"
                                         : L"🍉";
}

Food *newFood(void)
{
    Food *food = (Food *)malloc(sizeof(Food));
    food->first = NULL;
    return food;
}

void updateFood(Food *food, Snake *snake, Scene *scene)
{
    if (listLen(food->first) >= 50)
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

void updateFoodToScene(Scene *scene, Food *food)
{
    for (Node *node = food->first; node != NULL; node = node->next)
    {
        scene->buffer[node->y][node->x].ch = getFoodCh(scene, node->data);
        scene->buffer[node->y][node->x].color = scene->mode == RM_BG ? BACKGROUND_RED : 0;
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
