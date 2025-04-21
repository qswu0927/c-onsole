#include "snake.h"
#include <stdbool.h>
#include "../core/console.h"
#include "food.h"

Snake *newSnake(SHORT x, SHORT y)
{
    Snake *snake = (Snake *)malloc(sizeof(Snake));
    Node *node = (Node *)malloc(sizeof(Node));
    node->x = x;
    node->y = y;
    node->next = NULL;
    snake->tail = node;
    snake->dir = DIR_NONE;
    snake->alive = true;
    snake->score = 0;
    return snake;
}

void startSnake(void *context)
{
    Snake *snake = (Snake *)context;
    snake->dir = DIR_RIGHT;
}

void updateSnake(Snake *snake, Food *food, Scene *scene)
{
    if (snake->dir == DIR_NONE)
        return;
    // 找到脖子节点
    Node *neckNode = snake->tail;
    while (neckNode->next != NULL && neckNode->next->next != NULL)
    {
        neckNode = neckNode->next;
    }
    // 找到头节点，考虑只有一个节点的情况
    Node *headNode = neckNode->next == NULL ? neckNode : neckNode->next;

    SHORT x = headNode->x;
    SHORT y = headNode->y;
    if (snake->dir == DIR_UP)
        y--;
    else if (snake->dir == DIR_DOWN)
        y++;
    else if (snake->dir == DIR_LEFT)
        x--;
    else if (snake->dir == DIR_RIGHT)
        x++;

    // 吃到食物
    SHORT eat = eatFood(food, x, y);
    if (eat >= 0)
    {
        snake->score++;
        // 播放音效
        Beep(600, 5);
        Node *newNode = (Node *)malloc(sizeof(Node));
        newNode->data = eat;
        // 新节点位置是原头部位置
        newNode->x = headNode->x;
        newNode->y = headNode->y;
        newNode->next = headNode;
        // 头部位置更新
        headNode->x = x;
        headNode->y = y;
        if (neckNode != headNode)
            neckNode->next = newNode;
        else
            snake->tail = newNode;
    }
    else
    {
        // 从尾到头遍历链表，更新蛇的位置
        Node *node = snake->tail;
        for (; node->next != NULL; node = node->next)
        {
            node->x = node->next->x;
            node->y = node->next->y;
            // 判断撞到自己
            if (snake->alive && node->x == x && node->y == y)
                snake->alive = false;
        }
        // 更新头节点
        node->x = x;
        node->y = y;

        // 判断撞墙
        if (x <= 0 || x >= scene->w - 1 || y <= 0 || y >= scene->h - 1)
            snake->alive = false;
    }
}

wchar_t *getHeadCh(Scene *scene)
{
    if (scene->mode == RM_BG)
        return L"  ";
    if (scene->mode == RM_CH)
        return L"  ";
    return L"🤢";
}

void updateSnakeToScene(Scene *scene, Snake *snake)
{
    for (Node *node = snake->tail; node != NULL; node = node->next)
    {
        scene->buffer[node->y][node->x].ch = node->next == NULL ? getHeadCh(scene) : getFoodCh(scene, node->data);

        if (vtSupported)
        {
            RGBColor color = getFoodRgb(scene, node->data);
            // 蛇身绿色背景
            color.bg |= 0xaa;
            scene->buffer[node->y][node->x].color.rgb = node->next == NULL ? (scene->mode == RM_EM ? (RGBColor){0} : (RGBColor){0, 0, 0, 255, 255, 0}) : color;
        }
        else
        {
            scene->buffer[node->y][node->x].color.attr = node->next == NULL ? (scene->mode == RM_BG ? BACKGROUND_RED | BACKGROUND_GREEN : FOREGROUND_GREEN) : BACKGROUND_GREEN;
        }

        if (node->next == NULL)
        {
            // 摄像机跟随
            scene->camera.x = node->x - scene->camera.w / 2;
            scene->camera.y = node->y - scene->camera.h / 2;

            // 限位
            if (scene->camera.x < 0)
                scene->camera.x = 0;
            if (scene->camera.y < 0)
                scene->camera.y = 0;
            if (scene->camera.x + scene->camera.w > scene->w)
                scene->camera.x = scene->w - scene->camera.w;
            if (scene->camera.y + scene->camera.h > scene->h)
                scene->camera.y = scene->h - scene->camera.h;
        }
    }
}

void freeSnake(Snake **snakePtr)
{
    if (snakePtr == NULL || *snakePtr == NULL)
        return;
    Snake *snake = *snakePtr;
    while (snake->tail != NULL)
    {
        Node *current = snake->tail;
        snake->tail = snake->tail->next;
        free(current);
    }
    free(snake);
    *snakePtr = NULL;
}
