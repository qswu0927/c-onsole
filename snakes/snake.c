#include "snake.h"
#include <stdbool.h>
#include "../core/console.h"
#include "../core/music.h"
#include "food.h"

Snake *newSnake(Object *map, bool npc)
{
    SHORT x = rand() % (map->w - 2) + 1;
    SHORT y = rand() % (map->h - 2) + 1;
    // NPC从四周边缘进场
    Direction dir = rand() % 4;
    if (dir == DIR_UP)
        y = map->h - 1;
    else if (dir == DIR_DOWN)
        y = 0;
    else if (dir == DIR_LEFT)
        x = map->w - 1;
    else if (dir == DIR_RIGHT)
        x = 0;

    Snake *snake = (Snake *)malloc(sizeof(Snake));
    snake->tail = NULL;
    snake->dir = dir;
    snake->setDir = dir;
    snake->speed = SPEED_L / 2 + rand() % (SPEED_L * 2);
    snake->tick = 0;
    snake->alive = true;
    snake->score = 0;
    snake->npc = npc;
    snake->focus = false;
    // 玩家
    if (!npc)
    {
        dir = DIR_RIGHT;
        x = map->w / 2;
        y = map->h / 2;
        snake->dir = DIR_NONE;
        snake->setDir = DIR_NONE;
        snake->speed = SPEED_L;
        snake->focus = true;
    }
    // 初始化一定长度的蛇身
    for (SHORT i = 0; i < rand() % 3 + 2; i++)
    {
        Node *node = (Node *)malloc(sizeof(Node));
        node->x = x;
        node->y = y;
        if (dir == DIR_UP)
            node->y += i;
        else if (dir == DIR_DOWN)
            node->y -= i;
        else if (dir == DIR_LEFT)
            node->x += i;
        else if (dir == DIR_RIGHT)
            node->x -= i;
        node->data = rand() % 4;
        if (snake->tail == NULL)
        {
            snake->tail = node;
            node->next = NULL;
        }
        else
        {
            node->next = snake->tail;
            snake->tail = node;
        }
    }
    return snake;
}

void startSnake(void *context)
{
    Snake *snake = (Snake *)context;
    snake->setDir = DIR_RIGHT;
}

void autoDir(Snake **snakes, SHORT snakeCount, SHORT index, Object *map, Food *food)
{
    Node *neckNode = snakes[index]->tail;
    while (neckNode->next != NULL && neckNode->next->next != NULL)
        neckNode = neckNode->next;
    Node *headNode = neckNode->next == NULL ? neckNode : neckNode->next;

    if (snakes[index]->dir == DIR_NONE)
        snakes[index]->dir = rand() % 4;

    Direction dirs[3];
    SHORT dirWeight[3];
    dirs[2] = snakes[index]->dir;
    dirs[1] = (dirs[2] + 1) % 4;
    // dirs[0] = (dirs[2] + 2) % 4; // 不允许原地掉头
    dirs[0] = (dirs[2] + 3) % 4;
    // 大概率保持当前方向
    shuffle(dirs, rand() % 10 == 0 ? 3 : 2);

    for (SHORT i = 2; i >= 0; i--)
    {
        Direction dir = dirs[i];

        dirWeight[i] = 0;
        SHORT x = headNode->x;
        SHORT y = headNode->y;
        // 探测深度
        for (SHORT r = 5; r > 0; r--)
        {
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
            {
                dirWeight[i] -= r;
                break;
            }

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
            {
                dirWeight[i] -= r;
                break;
            }

            // 判断撞到其他蛇
            for (SHORT j = 0; j < snakeCount; j++)
            {
                if (j == index)
                    continue;
                if (snakes[j]->alive == false)
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
            {
                dirWeight[i] -= r;
                break;
            }

            // 判断吃到食物
            for (Node *node = food->first; node != NULL; node = node->next)
            {
                if (node->x == x && node->y == y)
                {
                    hit = true;
                    break;
                }
            }
            if (hit)
            {
                dirWeight[i] += r;
                continue;
            }
        }
    }

    // 取权重最大的方向
    SHORT maxWeight = dirWeight[2];
    SHORT maxIndex = 2;
    for (SHORT i = 1; i >= 0; i--)
    {
        if (dirWeight[i] > maxWeight)
        {
            maxWeight = dirWeight[i];
            maxIndex = i;
        }
    }
    snakes[index]->setDir = dirs[maxIndex];
}

void updateSnake(Snake **snakes, SHORT snakeCount, SHORT index, Food *food, Object *map)
{
    if (!snakes[index]->alive)
        return;

    if (snakes[index]->npc)
        autoDir(snakes, snakeCount, index, map, food);

    if (snakes[index]->tick++ < 12 / snakes[index]->speed)
        // 玩家方向变了，需要立即更新，避免输入延迟
        if (snakes[index]->npc || snakes[index]->dir == snakes[index]->setDir)
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
            if (snakes[j]->alive == false)
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
            playSoundSeq(3, snakes[index]->npc ? 350 : 450, 100, 0);
            snakes[index]->focus = false;
            // 转成食物
            for (Node *node = snakes[index]->tail; node != NULL; node = node->next)
            {
                // 避免正在穿墙的蛇转成食物卡在墙里
                if (node->x <= map->x || node->x >= map->x + map->w - 1 || node->y <= map->y || node->y >= map->y + map->h - 1)
                    continue;

                Node *fNode = (Node *)malloc(sizeof(Node));
                fNode->x = node->x;
                fNode->y = node->y;
                fNode->data = node->data;
                fNode->next = food->first;
                food->first = fNode;
            }
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
    if (snake->alive == false)
        return;
    for (Node *node = snake->tail; node != NULL; node = node->next)
    {
        if (node->x < scene->camera.x || node->x >= scene->camera.x + scene->camera.w || node->y < scene->camera.y || node->y >= scene->camera.y + scene->camera.h)
            continue;
        scene->buffer[node->y][node->x].ch = node->next == NULL ? getHeadCh(scene) : getFoodCh(scene, node->data);

        Rgb color = getFoodRgb(scene, node->data);
        // 蛇身背景
        if (snake->npc)
            color.br |= 0x99;
        else
            color.bg |= 0xaa;
        if (snake->focus)
            color.bb |= 0xcc;
        scene->buffer[node->y][node->x].color.rgb = node->next == NULL ? (scene->mode & RM_EM ? Rgb(50, 200, 0) : Rgbb(190, 200, 0)) : color;
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
