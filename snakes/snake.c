#include "snake.h"
#include <stdbool.h>
#include "../core/console.h"
#include "../core/music.h"
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
    snake->setDir = DIR_NONE;
    snake->tick = 0;
    snake->alive = true;
    snake->npc = true;
    snake->score = 0;
    return snake;
}

void startSnake(void *context)
{
    Snake *snake = (Snake *)context;
    snake->setDir = DIR_RIGHT;
}

void autoDir(Snake **snakes, SHORT snakeCount, SHORT index, Object *map)
{
    Node *neckNode = snakes[index]->tail;
    while (neckNode->next != NULL && neckNode->next->next != NULL)
        neckNode = neckNode->next;
    Node *headNode = neckNode->next == NULL ? neckNode : neckNode->next;

    if (snakes[index]->dir == DIR_NONE)
        snakes[index]->setDir = rand() % 4;

    Direction dir;
    Direction dirs[3];
    dirs[2] = snakes[index]->dir;
    dirs[1] = (dirs[2] + 1) % 4;
    // dirs[0] = (dirs[2] + 2) % 4; // 不允许原地掉头
    dirs[0] = (dirs[2] + 3) % 4;
    // 大概率保持当前方向
    shuffle(dirs, rand() % 10 == 0 ? 3 : 2);

    for (SHORT i = 2; i >= 0; i--)
    {
        dir = dirs[i];

        SHORT x = headNode->x;
        SHORT y = headNode->y;
        if (dir == DIR_UP)
            y--;
        else if (dir == DIR_DOWN)
            y++;
        else if (dir == DIR_LEFT)
            x--;
        else if (dir == DIR_RIGHT)
            x++;

        // 判断撞墙
        if (x <= 0 || x >= map->w - 1 || y <= 0 || y >= map->h - 1)
            continue;

        bool hit = false;
        // 判断撞到自己
        for (Node *node = snakes[index]->tail; node->next != NULL; node = node->next)
        {
            if (node->x == x && node->y == y)
            {
                hit = true;
                break;
            }
        }
        if (hit)
            continue;

        // 判断撞到其他蛇
        for (SHORT j = 0; j < snakeCount; j++)
        {
            if (j == index)
                continue;
            for (Node *node = snakes[j]->tail; node != NULL; node = node->next)
            {
                if (node->x == x && node->y == y)
                {
                    hit = true;
                    break;
                }
            }
            if (hit)
                break;
        }
        if (hit)
            continue;

        break;
    }
    snakes[index]->setDir = dir;
}

void updateSnake(Snake **snakes, SHORT snakeCount, SHORT index, Food *food, Object *map)
{
    if (!snakes[index]->alive)
        return;

    if (snakes[index]->npc)
        autoDir(snakes, snakeCount, index, map);

    // 方向变了，需要立即更新，避免玩家输入延迟
    if (snakes[index]->dir == snakes[index]->setDir && snakes[index]->tick++ < 12 / snakes[index]->speed)
        return;

    snakes[index]->tick = 0;
    snakes[index]->dir = snakes[index]->setDir;

    if (snakes[index]->dir == DIR_NONE)
        return;

    // 找到脖子节点
    Node *neckNode = snakes[index]->tail;
    while (neckNode->next != NULL && neckNode->next->next != NULL)
        neckNode = neckNode->next;

    // 找到头节点，考虑只有一个节点的情况
    Node *headNode = neckNode->next == NULL ? neckNode : neckNode->next;

    SHORT x = headNode->x;
    SHORT y = headNode->y;
    if (snakes[index]->dir == DIR_UP)
        y--;
    else if (snakes[index]->dir == DIR_DOWN)
        y++;
    else if (snakes[index]->dir == DIR_LEFT)
        x--;
    else if (snakes[index]->dir == DIR_RIGHT)
        x++;

    // 吃到食物
    SHORT eat = eatFood(food, x, y);
    if (eat >= 0)
    {
        snakes[index]->score++;
        playSoundSeq(3, 400 + eat * 200, 5, 0);
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
            snakes[index]->tail = newNode;
    }
    else
    {
        // 判断撞墙
        if (x <= 0 || x >= map->w - 1 || y <= 0 || y >= map->h - 1)
            snakes[index]->alive = false;

        // 判断撞到自己
        for (Node *node = snakes[index]->tail; node->next != NULL; node = node->next)
        {
            if (node->x == x && node->y == y)
            {
                snakes[index]->alive = false;
                break;
            }
        }

        // 判断撞到其他蛇
        for (SHORT j = 0; j < snakeCount; j++)
        {
            if (j == index)
                continue;
            for (Node *node = snakes[j]->tail; node != NULL; node = node->next)
            {
                if (node->x == x && node->y == y)
                {
                    snakes[index]->alive = false;
                    break;
                }
            }
            if (!snakes[index]->alive)
                break;
        }

        if (snakes[index]->alive)
        {
            // 从尾到头遍历链表，更新蛇的位置
            Node *node = snakes[index]->tail;
            for (; node->next != NULL; node = node->next)
            {
                node->x = node->next->x;
                node->y = node->next->y;
            }
            // 更新头节点
            node->x = x;
            node->y = y;
        }
        else
        {
            playSoundSeq(3, snakes[index]->npc ? 300 : 350, 100, 0);
        }
    }
}

wchar_t *getHeadCh(Scene *scene)
{
    if (scene->mode & RM_BG)
        return _wcsdup(L"  ");
    if (scene->mode & RM_CH)
        return _wcsdup(L"  ");
    return _wcsdup(L"🤢");
}

void setSceneSnake(Scene *scene, Snake *snake)
{
    for (Node *node = snake->tail; node != NULL; node = node->next)
    {
        scene->buffer[node->y][node->x].ch = node->next == NULL ? getHeadCh(scene) : getFoodCh(scene, node->data);

        if (scene->mode & RM_V)
        {
            Rgb color = getFoodRgb(scene, node->data);
            // 蛇身背景
            if (snake->npc)
                color.br |= 0x99;
            else
                color.bg |= 0xaa;
            scene->buffer[node->y][node->x].color.rgb = node->next == NULL ? (scene->mode & RM_EM ? (Rgb){0} : (Rgb){0, 0, 0, 255, snake->npc ? 100 : 255, 0}) : color;
        }
        else
        {
            scene->buffer[node->y][node->x].attrSet = true;
            scene->buffer[node->y][node->x].color.attr = node->next == NULL ? (scene->mode & RM_BG ? BACKGROUND_RED | BACKGROUND_GREEN : FOREGROUND_GREEN) : BACKGROUND_GREEN;
        }
    }
}

SHORT getAliveCount(Snake **snakes, SHORT snakeCount)
{
    SHORT count = 0;
    for (SHORT i = 0; i < snakeCount; i++)
    {
        if (snakes[i]->alive)
            count++;
    }
    return count;
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
