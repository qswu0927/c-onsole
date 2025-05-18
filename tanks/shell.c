#include "shell.h"
#include <windows.h>
#include "../core/console.h"
#include "../core/music.h"
#include "map.h"
#include "tank.h"

bool isTankHit(SHORT x0, SHORT y0, Tank *tank)
{
    if (tank->health <= 0)
        return false;

    for (SHORT y = 0; y < tank->h; y++)
    {
        for (SHORT x = 0; x < tank->w; x++)
        {
            if (tank->data[y][x] == 0)
                continue;
            if (tank->x + x == x0 && tank->y + y == y0)
                return true;
        }
    }
    return false;
}

void fireShell(Shell **shellPtr, Tank *tank, Object *map, Scene *scene)
{
    Shell *shell = *shellPtr;
    Shell *newShell = (Shell *)malloc(sizeof(Shell));
    newShell->x = tank->x + tank->w / 2;
    newShell->y = tank->y + tank->h / 2;
    newShell->data = 1;
    newShell->vx = 0;
    newShell->vy = 0;
    newShell->damage = 20;
    newShell->owner = tank;
    if (tank->dir == DIR_UP)
    {
        newShell->vy = -1;
        newShell->y = tank->y;
    }
    else if (tank->dir == DIR_DOWN)
    {
        newShell->vy = 1;
        newShell->y = tank->y + tank->h - 1;
    }
    else if (tank->dir == DIR_LEFT)
    {
        newShell->vx = -1;
        newShell->x = tank->x;
    }
    else if (tank->dir == DIR_RIGHT)
    {
        newShell->vx = 1;
        newShell->x = tank->x + tank->w - 1;
    }

    newShell->next = shell;
    *shellPtr = newShell;
}

DWORD WINAPI explodeShell(LPVOID lpParam)
{
    ExplodeParams *params = (ExplodeParams *)lpParam;
    Shell *shell = params->shell;
    Tank **tanks = params->tanks;
    SHORT tankCount = params->tankCount;
    Object *map = params->map;
    Object *frags = NULL;

    SHORT rowBegin = -1, rowEnd = 1, colBegin = -1, colEnd = 1;
    if (shell->vx < 0)
        colBegin = -2;
    if (shell->vx > 0)
        colEnd = 2;
    if (shell->vy < 0)
        rowBegin = -2;
    if (shell->vy > 0)
        rowEnd = 2;

    for (SHORT r = rowBegin; r <= rowEnd; r++)
    {
        for (SHORT c = colBegin; c <= colEnd; c++)
        {
            SHORT r0 = shell->y + r;
            SHORT c0 = shell->x + c;
            if (r0 <= 0 || r0 >= map->h - 1 || c0 <= 0 || c0 >= map->w - 1)
                continue;

            CHAR ch = -1;
            for (SHORT i = 0; i < tankCount; i++)
            {
                if (tanks[i]->health <= 0)
                    continue;
                if (isTankHit(c0, r0, tanks[i]))
                {
                    ch = tanks[i]->data[r0 - tanks[i]->y][c0 - tanks[i]->x];
                    tanks[i]->health -= shell->damage;
                    if (tanks[i]->health <= 0)
                    {
                        tanks[i]->health = 0;
                        // playSoundSeq(3, tanks[i]->npc ? 350 : 450, 100, 0);
                        tanks[i]->focus = false;
                        tanks[i]->killBy = shell->owner;
                    }
                    shell->owner->score += shell->damage;
                    break;
                }
            }

            if (ch < 0 && map->data[r0][c0] != 0)
            {
                ch = map->data[r0][c0];
                map->data[r0][c0] = 0;
            }
            if (ch > 0)
            {
                Object *frag = newObject(c0, r0, 1, 1);
                frag->data[0][0] = ch;
                frag->vx = c;
                frag->vy = r;
                frag->status = 0;
                if (frags)
                {
                    if (frags->next != NULL)
                        frag->next = frags->next;
                    frags->next = frag;
                }
                else
                    frags = frag;
            }
        }
    }
    free(shell);
    playSoundSeq(10 * 3,
                 // 中高频起始
                 350, 30, 0,
                 300, 30, 0,
                 250, 30, 0,
                 200, 30, 0,
                 150, 30, 0,
                 100, 30, 0,
                 // 爆炸冲击波主体
                 80, 100, 0, // 低频轰隆声
                 60, 150, 0, // 更低的余震
                 // 爆炸结束的"火花"声
                 120, 40, 0,
                 200, 30, 0);

    while (frags)
    {
        Object *f = frags;
        Object *fp = NULL;
        while (f)
        {
            f->x += f->vx;
            f->y += f->vy;
            f->status++;
            if ((f->vx == 0 && f->vy == 0) || f->status > 5 || f->x <= map->x || f->x >= map->x + map->w - 1 || f->y <= map->y || f->y >= map->y + map->h - 1 || map->data[f->y][f->x] > 0)
            {
                if (fp == NULL)
                {
                    frags = f->next;
                    freeObject(&f);
                    f = frags;
                }
                else
                {
                    fp->next = f->next;
                    freeObject(&f);
                    f = fp->next;
                }
                continue;
            }
            fp = f;
            f = f->next;
        }
        map->next = frags;
        Sleep(40);
    }
    free(params);
    return 0;
}

void updateShell(Shell **shellPtr, Tank **tanks, SHORT tankCount, Object *map)
{
    Shell *shell = *shellPtr;
    for (Shell *node = shell, *prev = NULL; node != NULL;)
    {
        node->x += node->vx;
        node->y += node->vy;
        node->damage--;

        bool hit = false;
        for (SHORT i = 0; i < tankCount; i++)
        {
            // 击中坦克
            if (isTankHit(node->x, node->y, tanks[i]))
            {
                hit = true;
                break;
            }
        }
        // 击中墙体
        if (!hit && node->x > map->x && node->x < map->x + map->w - 1 &&
            node->y > map->y && node->y < map->y + map->h - 1 && map->data[node->y][node->x] != 0)
        {
            hit = true;
        }
        if (hit && node->damage > 0)
        {
            ExplodeParams *params = (ExplodeParams *)malloc(sizeof(ExplodeParams));
            params->shell = node;
            params->tanks = tanks;
            params->tankCount = tankCount;
            params->map = map;
            HANDLE hThread = CreateThread(NULL, 0, explodeShell, params, 0, NULL);
            if (hThread)
                CloseHandle(hThread);
        }
        // 删除炮弹
        if (hit || node->damage <= 0 || node->x <= map->x || node->x >= map->x + map->w - 1 ||
            node->y <= map->y || node->y >= map->y + map->h - 1 || map->data[node->y][node->x] != 0)
        {
            if (prev == NULL)
            {
                *shellPtr = node->next;
                if (!hit)
                    free(node);
                node = *shellPtr;
            }
            else
            {
                prev->next = node->next;
                if (!hit)
                    free(node);
                node = prev->next;
            }
            continue;
        }
        prev = node;
        node = node->next;
    }
}

Rgb getShellRgb(Scene *scene, SHORT data)
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

void setSceneShell(Scene *scene, Shell *shell)
{
    for (Shell *node = shell; node != NULL; node = node->next)
    {
        scene->buffer[node->y][node->x].ch = getCh(scene, node->data);
        scene->buffer[node->y][node->x].color.rgb = getShellRgb(scene, node->data);
    }
}

void setSceneFrag(Scene *scene, Object *frag)
{
    for (Object *node = frag; node != NULL; node = node->next)
    {
        scene->buffer[node->y][node->x].ch = getCh(scene, node->data[0][0]);
        scene->buffer[node->y][node->x].color.rgb = getShellRgb(scene, node->data[0][0]);
    }
}

void freeShell(Shell **shellPtr)
{
    if (!shellPtr || !*shellPtr)
        return;
    Shell *shell = *shellPtr;
    while (shell != NULL)
    {
        Shell *next = shell->next;
        free(shell);
        shell = next;
    }
    *shellPtr = NULL;
}
