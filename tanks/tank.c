#include "tank.h"
#include <stdbool.h>
#include "../core/console.h"
#include "../core/music.h"
#include "map.h"
#include "shell.h"

const SHORT TANKSHAPE[TANK_HEIGHT][TANK_WIDTH] = {{0, 30, 0}, {31, 30, 31}, {32, 0, 32}};

void rotateTank(Tank *tank)
{
    SHORT w = tank->w;
    SHORT h = tank->h;
    CHAR **rotatedData = (CHAR **)malloc(w * sizeof(CHAR *));
    for (SHORT i = 0; i < w; i++)
        rotatedData[i] = (CHAR *)malloc(h * sizeof(CHAR));

    rotate90Clockwise(h, w, tank->data, rotatedData);

    // 释放原数据
    for (int i = 0; i < h; i++)
        free(tank->data[i]);
    free(tank->data);

    tank->data = rotatedData;
    tank->w = h;
    tank->h = w;
    tank->dir = ++tank->dir % 4;
}

Tank *newTank(SHORT id, Object *map, Scene *scene, bool npc)
{
    SHORT x0, y0;
    while (true)
    {
        x0 = rand() % (map->w - TANK_WIDTH + 1);
        y0 = rand() % (map->h - TANK_HEIGHT + 1);
        bool valid = true;
        for (SHORT y = y0; y < y0 + TANK_HEIGHT; y++)
        {
            for (SHORT x = x0; x < x0 + TANK_WIDTH; x++)
            {
                if (map->data[y][x] != 0)
                {
                    valid = false;
                    break;
                }
                if (x >= scene->camera.x && x < scene->camera.x + scene->camera.w && y >= scene->camera.y && y < scene->camera.y + scene->camera.h)
                {
                    // NPC刷新在视野外
                    if (npc)
                    {
                        valid = false;
                        break;
                    }
                }
                else
                {
                    // 玩家刷新在视野内
                    if (!npc)
                    {
                        valid = false;
                        break;
                    }
                }
            }
            if (!valid)
                break;
        }
        if (valid)
            break;
    }

    Tank *tank = (Tank *)malloc(sizeof(Tank));
    tank->id = id;
    tank->x = x0;
    tank->y = y0;
    tank->w = TANK_WIDTH;
    tank->h = TANK_HEIGHT;
    tank->data = (CHAR **)malloc(tank->h * sizeof(CHAR *));
    for (SHORT y = 0; y < tank->h; y++)
    {
        tank->data[y] = (CHAR *)malloc(tank->w * sizeof(CHAR));
        for (SHORT x = 0; x < tank->w; x++)
            tank->data[y][x] = TANKSHAPE[y][x];
    }
    tank->dir = DIR_UP;
    tank->interMoveMs = rand() % 1000;
    tank->lastMoveQpc = 0;
    tank->interLoadShellMs = 1500;
    tank->lastLoadShellQpc = 0;
    tank->maxShellCount = 10;
    tank->maxHealth = 100;
    tank->score = 0;
    tank->killerId = -1;
    tank->npc = npc;
    tank->focus = false;
    if (!npc)
    {
        tank->interMoveMs = INT_MAX;
        tank->interLoadShellMs = 1200;
        tank->maxHealth = 500;
        tank->focus = true;
    }
    tank->shellCount = tank->maxShellCount;
    tank->health = tank->maxHealth;
    // 随机旋转
    for (SHORT r = 0; r < rand() % 4; r++)
        rotateTank(tank);
    tank->setDir = tank->dir;
    return tank;
}

bool isMapCollided(Tank *tank, Object *map)
{
    for (SHORT y = 0; y < tank->h; y++)
    {
        for (SHORT x = 0; x < tank->w; x++)
        {
            if (tank->data[y][x] == 0)
                continue;
            if (tank->x + x < map->x || tank->x + x >= map->x + map->w || tank->y + y < map->y || tank->y + y >= map->y + map->h)
                continue;
            if (map->data[tank->y + y][tank->x + x] != 0)
                return true;
        }
    }
    return false;
}

bool isTankCollided(Tank *tank, Tank *tank2)
{
    if (tank->health <= 0 || tank2->health <= 0)
        return false;

    for (SHORT y = 0; y < tank->h; y++)
    {
        for (SHORT x = 0; x < tank->w; x++)
        {
            if (tank->data[y][x] == 0)
                continue;
            for (SHORT y2 = 0; y2 < tank2->h; y2++)
            {
                for (SHORT x2 = 0; x2 < tank2->w; x2++)
                {
                    if (tank2->data[y2][x2] == 0)
                        continue;
                    if (tank->x + x == tank2->x + x2 && tank->y + y == tank2->y + y2)
                        return true;
                }
            }
        }
    }
    return false;
}

bool isCollided(Tank **tanks, SHORT tankCount, SHORT index, Object *map)
{
    for (SHORT i = 0; i < tankCount; i++)
    {
        if (i == index)
            continue;
        if (isTankCollided(tanks[index], tanks[i]))
            return true;
    }
    return isMapCollided(tanks[index], map);
}

bool canRotate(Tank **tanks, SHORT tankCount, SHORT index, Object *map)
{
    if (!isCollided(tanks, tankCount, index, map))
        return true;

    // 墙踢（Wall Kick）：当旋转后碰到墙壁或障碍物时，系统会尝试微调方块位置以允许旋转
    for (short offset = 1; offset <= tanks[index]->w / 2; offset++)
    {
        tanks[index]->x -= offset;
        if (!isCollided(tanks, tankCount, index, map))
            return true;
        tanks[index]->x += 2 * offset;
        if (!isCollided(tanks, tankCount, index, map))
            return true;
        tanks[index]->x -= offset;
    }
    for (short offset = 1; offset <= tanks[index]->h / 2; offset++)
    {
        tanks[index]->y -= offset;
        if (!isCollided(tanks, tankCount, index, map))
            return true;
        tanks[index]->y += 2 * offset;
        if (!isCollided(tanks, tankCount, index, map))
            return true;
        tanks[index]->y -= offset;
    }
    return false;
}

void autoPilot(Tank **tanks, SHORT tankCount, SHORT index, Shell **shellPtr, Object *map, Scene *scene)
{
    if (tanks[index]->dir == DIR_NONE)
        tanks[index]->dir = rand() % 4;

    Direction dirs[4];
    SHORT dirWeight[4];
    SHORT fireWeight[4];
    dirs[3] = tanks[index]->dir;
    dirs[2] = (dirs[3] + 1) % 4;
    dirs[1] = (dirs[3] + 2) % 4;
    dirs[0] = (dirs[3] + 3) % 4;
    // 大概率保持当前方向
    shuffle(dirs, rand() % 10 == 0 ? 4 : 3);

    SHORT x = tanks[index]->x;
    SHORT y = tanks[index]->y;
    for (SHORT i = 3; i >= 0; i--)
    {
        Direction dir = dirs[i];

        dirWeight[i] = 0;
        fireWeight[i] = 0;
        tanks[index]->x = x;
        tanks[index]->y = y;
        // 探测深度
        for (SHORT r = 15; r > 0; r--)
        {
            if (dir == DIR_UP)
                tanks[index]->y--;
            else if (dir == DIR_DOWN)
                tanks[index]->y++;
            else if (dir == DIR_LEFT)
                tanks[index]->x--;
            else if (dir == DIR_RIGHT)
                tanks[index]->x++;

            if (isMapCollided(tanks[index], map))
            {
                // dirWeight[i] -= r;
                // break;
            }
            for (SHORT j = 0; j < tankCount; j++)
            {
                if (j == index)
                    continue;
                if (isTankCollided(tanks[index], tanks[j]))
                {
                    // if (r > 10)
                    //     dirWeight[i] -= r;
                    // else
                    dirWeight[i] += r;
                    fireWeight[i] += r;
                    break;
                }
            }
            for (Shell *node = *shellPtr; node != NULL; node = node->next)
            {
                if (node->ownerId == index)
                    continue;
                if (node->vx < 0 && node->x < tanks[index]->x)
                    continue;
                if (node->vx > 0 && node->x > tanks[index]->x)
                    continue;
                if (node->vy < 0 && node->y < tanks[index]->y)
                    continue;
                if (node->vy > 0 && node->y > tanks[index]->y)
                    continue;
                if (isTankHit(node->x, node->y, tanks[index]))
                {
                    // dirWeight[i] -= r;
                    dirWeight[i] -= node->damage;
                }
            }
        }
    }
    tanks[index]->x = x;
    tanks[index]->y = y;

    // 取权重最大的方向
    SHORT maxDirWeight = dirWeight[3];
    SHORT maxDirIndex = 3;
    SHORT maxFireWeight = fireWeight[3];
    SHORT maxFireIndex = 3;
    for (SHORT i = 2; i >= 0; i--)
    {
        if (dirWeight[i] > maxDirWeight)
        {
            maxDirWeight = dirWeight[i];
            maxDirIndex = i;
        }
        if (fireWeight[i] > maxFireWeight)
        {
            maxFireWeight = fireWeight[i];
            maxFireIndex = i;
        }
    }
    // 小概率随机方向，避免两玩家出现死锁
    if (rand() % 10 == 0)
    {
        maxDirIndex = rand() % 4;
    }
    tanks[index]->setDir = dirs[maxDirIndex];
    if (maxFireIndex == maxDirIndex && maxFireWeight > 30 && rand() % 8 == 0)
        fireShell(shellPtr, tanks[index]);
}

void updateTank(Tank **tanks, SHORT tankCount, SHORT index, Shell **shellPtr, Object *map, Scene *scene)
{
    if (tanks[index]->health <= 0 && tanks[index]->npc)
    {
        if (rand() % 100 == 0)
        {
            freeTank(&tanks[index]);
            tanks[index] = newTank(index, map, scene, true);
        }
        return;
    }

    if (tanks[index]->npc)
        autoPilot(tanks, tankCount, index, shellPtr, map, scene);

    if (tanks[index]->shellCount < tanks[index]->maxShellCount && (double)(fr.start.QuadPart - tanks[index]->lastLoadShellQpc) * 1000.0 / fr.frequency.QuadPart > tanks[index]->interLoadShellMs)
    {
        tanks[index]->shellCount++;
        tanks[index]->lastLoadShellQpc = fr.start.QuadPart;
    }

    SHORT x = tanks[index]->x;
    SHORT y = tanks[index]->y;
    Direction dir = tanks[index]->dir;
    if (tanks[index]->dir != tanks[index]->setDir)
    {
        while (tanks[index]->dir != tanks[index]->setDir)
            rotateTank(tanks[index]);
        if (!canRotate(tanks, tankCount, index, map))
        {
            // 转回来
            while (tanks[index]->dir != dir)
                rotateTank(tanks[index]);
            tanks[index]->x = x;
            tanks[index]->y = y;
            tanks[index]->setDir = tanks[index]->dir;
        }
    }
    else if ((double)(fr.start.QuadPart - tanks[index]->lastMoveQpc) * 1000.0 / fr.frequency.QuadPart > tanks[index]->interMoveMs)
    {
        tanks[index]->lastMoveQpc = fr.start.QuadPart;

        if (tanks[index]->dir == DIR_UP)
            tanks[index]->y--;
        else if (tanks[index]->dir == DIR_DOWN)
            tanks[index]->y++;
        else if (tanks[index]->dir == DIR_LEFT)
            tanks[index]->x--;
        else if (tanks[index]->dir == DIR_RIGHT)
            tanks[index]->x++;

        if (isCollided(tanks, tankCount, index, map))
        {
            tanks[index]->x = x;
            tanks[index]->y = y;
        }
    }
}

void setSceneTank(Scene *scene, Tank *tank)
{
    if (tank->health <= 0)
        return;
    for (SHORT y = 0; y < tank->h; y++)
    {
        for (SHORT x = 0; x < tank->w; x++)
        {
            if (tank->data[y][x] == 0)
                continue;
            if (tank->x + x < scene->camera.x || tank->x + x >= scene->camera.x + scene->camera.w || tank->y + y < scene->camera.y || tank->y + y >= scene->camera.y + scene->camera.h)
                continue;
            scene->buffer[tank->y + y][tank->x + x].ch = getCh(scene, tank->data[y][x]);
            Rgb color = getRgb(scene, tank->data[y][x], 0, 0);
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

void freeTank(Tank **tankPtr)
{
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
    } while (tanks[tmp]->health <= 0 && tmp != *focusOn);
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
        if (tanks[i]->health <= 0)
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
