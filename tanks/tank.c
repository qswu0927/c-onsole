#include "tank.h"
#include <stdbool.h>
#include "../core/console.h"
#include "../core/music.h"
#include "shell.h"

Tank *newTank(Object *map, bool npc)
{
    SHORT x = rand() % (map->w - 2) + 1;
    SHORT y = rand() % (map->h - 2) + 1;
    Direction dir = rand() % 4;
    if (dir == DIR_UP)
        y = map->h - 1;
    else if (dir == DIR_DOWN)
        y = 0;
    else if (dir == DIR_LEFT)
        x = map->w - 1;
    else if (dir == DIR_RIGHT)
        x = 0;

    Tank *tank = (Tank *)malloc(sizeof(Tank));
    tank->x = x;
    tank->y = y;
    tank->w = 3;
    tank->h = 3;
    tank->data = (CHAR **)malloc(tank->h * sizeof(CHAR *));
    for (SHORT i = 0; i < tank->h; i++)
    {
        tank->data[i] = (CHAR *)malloc(tank->w * sizeof(CHAR));
        for (SHORT j = 0; j < tank->w; j++)
            tank->data[i][j] = i == 0 && j != 1 ? 0 : 1;
    }
    tank->dir = dir;
    tank->setDir = dir;
    tank->speed = SPEED_L / 2 + rand() % (SPEED_L * 2);
    tank->tick = 0;
    tank->alive = true;
    tank->score = 0;
    tank->npc = npc;
    tank->focus = false;
    // 玩家
    if (!npc)
    {
        dir = DIR_RIGHT;
        x = map->w / 2;
        y = map->h / 2;
        tank->dir = DIR_NONE;
        tank->setDir = DIR_NONE;
        tank->speed = SPEED_L;
        tank->focus = true;
    }
    return tank;
}

bool isTankCollided(Tank *tank1, Tank *tank2)
{
    for (SHORT y1 = 0; y1 < tank1->h; y1++)
    {
        for (SHORT x1 = 0; x1 < tank1->w; x1++)
        {
            if (tank1->data[y1][x1] == 0)
                continue;
            for (SHORT y2 = 0; y2 < tank2->h; y2++)
            {
                for (SHORT x2 = 0; x2 < tank2->w; x2++)
                {
                    if (tank2->data[y2][x2] == 0)
                        continue;
                    if (tank1->x + x1 == tank2->x + x2 && tank1->y + y1 == tank2->y + y2)
                        return true;
                }
            }
        }
    }
    return false;
}

void autoDir(Tank **tanks, SHORT tankCount, SHORT index, Object *map, Shell *shell)
{
    if (tanks[index]->dir == DIR_NONE)
        tanks[index]->dir = rand() % 4;

    Direction dirs[4];
    SHORT dirWeight[4];
    dirs[3] = tanks[index]->dir;
    dirs[2] = (dirs[2] + 1) % 4;
    dirs[1] = (dirs[2] + 2) % 4;
    dirs[0] = (dirs[2] + 3) % 4;
    // 大概率保持当前方向
    shuffle(dirs, rand() % 10 == 0 ? 4 : 3);

    for (SHORT i = 2; i >= 0; i--)
    {
        Direction dir = dirs[i];

        dirWeight[i] = 0;
        SHORT x = tanks[index]->x;
        SHORT y = tanks[index]->y;
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
            // 判断撞到其他玩家
            for (SHORT j = 0; j < tankCount; j++)
            {
                if (j == index)
                    continue;
                if (tanks[j]->alive == false)
                    continue;
                hit = isTankCollided(tanks[index], tanks[j]);
                if (hit)
                    break;
            }
            if (hit)
            {
                dirWeight[i] -= r;
                break;
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
    tanks[index]->setDir = dirs[maxIndex];
}

void updateTank(Tank **tanks, SHORT tankCount, SHORT index, Shell *shell, Object *map, Scene *scene)
{
    if (!tanks[index]->alive)
        return;

    if (tanks[index]->npc)
        autoDir(tanks, tankCount, index, map, shell);

    if (tanks[index]->tick++ < 12 / tanks[index]->speed)
        // 玩家方向变了，需要立即更新，避免输入延迟
        if (tanks[index]->npc || tanks[index]->dir == tanks[index]->setDir)
            return;

    tanks[index]->tick = 0;
    tanks[index]->dir = tanks[index]->setDir;

    if (tanks[index]->dir == DIR_NONE)
        return;

    SHORT x = tanks[index]->x;
    SHORT y = tanks[index]->y;
    if (tanks[index]->dir == DIR_UP)
        y--;
    else if (tanks[index]->dir == DIR_DOWN)
        y++;
    else if (tanks[index]->dir == DIR_LEFT)
        x--;
    else if (tanks[index]->dir == DIR_RIGHT)
        x++;

    // 判断撞墙
    if (x <= 0 || x >= map->w - 1 || y <= 0 || y >= map->h - 1)
        tanks[index]->alive = false;

    // 判断撞到其他玩家
    for (SHORT j = 0; j < tankCount; j++)
    {
        if (j == index)
            continue;
        if (tanks[j]->alive == false)
            continue;
        tanks[index]->alive = !isTankCollided(tanks[index], tanks[j]);
        if (!tanks[index]->alive)
            break;
    }

    if (tanks[index]->alive)
    {
        tanks[index]->x = x;
        tanks[index]->y = y;
    }
    else
    {
        playSoundSeq(3, tanks[index]->npc ? 350 : 450, 100, 0);
        tanks[index]->focus = false;
    }
}

// wchar_t *getHeadCh(Scene *scene)
// {
//     if (scene->mode & RM_BG)
//         return _wcsdup(L"  ");
//     if (scene->mode & RM_CH)
//         return _wcsdup(L"  ");
//     return _wcsdup(L"🤢");
// }

void setSceneTank(Scene *scene, Tank *tank)
{
    if (tank->alive == false)
        return;
    for (SHORT y = 0; y < tank->h; y++)
    {
        for (SHORT x = 0; x < tank->w; x++)
        {
            if (tank->data[y][x] == 0)
                continue;
            if (tank->x + x < scene->camera.x || tank->x + x >= scene->camera.x + scene->camera.w || tank->y + y < scene->camera.y || tank->y + y >= scene->camera.y + scene->camera.h)
                continue;
            scene->buffer[tank->y + y][tank->x + x].ch = getShellCh(scene, tank->data[y][x]);
            Rgb color = getShellRgb(scene, tank->data[y][x]);
            if (tank->npc)
                color.br |= 0x99;
            else
                color.bg |= 0xaa;
            if (tank->focus)
                color.bb |= 0xcc;
            scene->buffer[tank->y + y][tank->x + x].color.rgb = color;
        }
    }
}

SHORT getAliveCount(Tank **tanks, SHORT tankCount)
{
    SHORT count = 0;
    for (SHORT i = 0; i < tankCount; i++)
        if (tanks[i]->alive)
            count++;
    return count;
}

void freeTank(Tank **tankPtr)
{
    if (tankPtr == NULL || *tankPtr == NULL)
        return;
    Tank *tank = *tankPtr;
    for (SHORT i = 0; i < tank->h; i++)
        if (tank->data[i])
            free(tank->data[i]);
    free(tank->data);
    free(tank);
    *tankPtr = NULL;
}

void changeFocus(Tank **tanks, SHORT tankCount, SHORT *focusOn)
{
    SHORT tmp = *focusOn;
    do
    {
        tmp++;
        if (tmp >= tankCount)
            tmp = 0;
    } while (tanks[tmp]->alive == false && tmp != *focusOn);
    tanks[*focusOn]->focus = false;
    *focusOn = tmp;
    tanks[*focusOn]->focus = true;
}

void changeFocusNear(Tank **tanks, SHORT tankCount, SHORT *focusOn)
{
    SHORT tmp = *focusOn;
    int min = INT_MAX;
    for (SHORT i = 0; i < tankCount; i++)
    {
        if (i == *focusOn)
            continue;
        if (tanks[i]->alive == false)
            continue;
        int dist = (tanks[*focusOn]->x - tanks[i]->x) * (tanks[*focusOn]->x - tanks[i]->x) + (tanks[*focusOn]->y - tanks[i]->y) * (tanks[*focusOn]->y - tanks[i]->y);
        if (dist < min)
        {
            min = dist;
            tmp = i;
        }
    }
    tanks[*focusOn]->focus = false;
    *focusOn = tmp;
    tanks[*focusOn]->focus = true;
}
