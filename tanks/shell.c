#include "shell.h"
#include <windows.h>
#include "../core/console.h"
#include "../core/music.h"
#include "map.h"
#include "tank.h"

void fireShell(Shell **shellPtr, Tank *tank)
{
    Shell *shell = *shellPtr;
    Shell *newShell = (Shell *)malloc(sizeof(Shell));
    newShell->x = tank->x + tank->w / 2;
    newShell->y = tank->y + tank->h / 2;
    newShell->data = 10;
    newShell->vx = 0;
    newShell->vy = 0;
    newShell->damage = SHELL_MAX_DAMAGE;
    newShell->owner = tank;
    if (tank->dir == DIR_UP)
        newShell->vy = -1;
    else if (tank->dir == DIR_DOWN)
        newShell->vy = 1;
    else if (tank->dir == DIR_LEFT)
        newShell->vx = -1;
    else if (tank->dir == DIR_RIGHT)
        newShell->vx = 1;

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

    // 伤害范围
    SHORT rowBegin = shell->vy - 1;
    SHORT rowEnd = shell->vy + 1;
    SHORT colBegin = shell->vx - 1;
    SHORT colEnd = shell->vx + 1;

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

    while (frags)
    {
        Object *f = frags;
        Object *fp = NULL;
        while (f)
        {
            f->x += f->vx;
            f->y += f->vy;
            f->status++;
            if ((f->vx == 0 && f->vy == 0) || f->status > 5 || f->x <= map->x || f->x >= map->x + map->w - 1 || f->y <= map->y || f->y >= map->y + map->h - 1 /* || map->data[f->y][f->x] > 0 */)
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

void updateShell(Shell **shellPtr, Tank **tanks, SHORT tankCount, Object *map, Scene *scene)
{
    Shell *shell = *shellPtr;
    for (Shell *node = shell, *prev = NULL; node != NULL;)
    {
        node->x += node->vx;
        node->y += node->vy;

        bool hit = false;
        for (SHORT i = 0; i < tankCount; i++)
        {
            // 不要击中自家
            if (node->owner == tanks[i])
                continue;
            // 击中坦克
            if (isTankHit(node->x, node->y, tanks[i]))
            {
                hit = true;
                // 玩家炮弹或击中玩家或在视野范围才播放音效
                if (!tanks[i]->npc || !node->owner->npc || (node->x >= scene->camera.x && node->x < scene->camera.x + scene->camera.w && node->y >= scene->camera.y && node->y < scene->camera.y + scene->camera.h))
                    playSoundSeq(3, 200 + shell->damage * 300 / SHELL_MAX_DAMAGE, 5, 0);
                break;
            }
        }
        // 击中墙体
        if (!hit && node->x > map->x && node->x < map->x + map->w - 1 &&
            node->y > map->y && node->y < map->y + map->h - 1 && map->data[node->y][node->x] != 0)
        {
            hit = true;
            // 玩家炮弹或在视野范围才播放音效
            if (!node->owner->npc || (node->x >= scene->camera.x && node->x < scene->camera.x + scene->camera.w && node->y >= scene->camera.y && node->y < scene->camera.y + scene->camera.h))
                playSoundSeq(3, 220, 10, 0);
        }
        if (hit)
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

        node->damage--;
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
        orColor(&scene->buffer[node->y][node->x].color.rgb, getShellRgb(scene, node->data));
    }
}

void setSceneFrag(Scene *scene, Object *frag)
{
    for (Object *node = frag; node != NULL; node = node->next)
    {
        scene->buffer[node->y][node->x].ch = getCh(scene, node->data[0][0]);
        orColor(&scene->buffer[node->y][node->x].color.rgb, getShellRgb(scene, node->data[0][0]));
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
