#include "block.h"
#include <time.h>
#include "../core/util.h"
#include "../core/console.h"
#include "../core/music.h"
#include "map.h"

// 基本方块
const SHORT TETROMINOS[][BLOCK_HEIGHT][BLOCK_WIDTH] = {
    // I
    {{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    // O
    {{0, 0, 0, 0}, {0, 2, 2, 0}, {0, 2, 2, 0}, {0, 0, 0, 0}},
    // T
    {{0, 0, 0, 0}, {0, 3, 0, 0}, {3, 3, 3, 0}, {0, 0, 0, 0}},
    // L
    {{0, 0, 0, 0}, {0, 0, 4, 0}, {4, 4, 4, 0}, {0, 0, 0, 0}},
    // J
    {{0, 0, 0, 0}, {5, 0, 0, 0}, {5, 5, 5, 0}, {0, 0, 0, 0}},
    // S
    {{0, 0, 0, 0}, {0, 6, 6, 0}, {6, 6, 0, 0}, {0, 0, 0, 0}},
    // Z
    {{0, 0, 0, 0}, {7, 7, 0, 0}, {0, 7, 7, 0}, {0, 0, 0, 0}}};
const SHORT NUM_TETROMINOS = sizeof(TETROMINOS) / sizeof(TETROMINOS[0]);

wchar_t *getBlockCh(SHORT index, Scene *scene)
{
    if (scene->mode == 0)
        return _wcsdup(L"  ");

    if (scene->mode == 1)
    {
        if (index == 0)
            return _wcsdup(L"  ");
        wchar_t ch[] = L"哈啊哦嗯哇呀啦炸";
        // 方式一
        wchar_t temp[] = {ch[index - 1], L'\0'};
        return _wcsdup(temp);
    }

    wchar_t ch[] = L"  😂😊🍉🌻🍁🍖🎈💣";
    // 方式二
    wchar_t *result = (wchar_t *)malloc(3 * sizeof(wchar_t));
    if (result)
    {
        result[0] = ch[2 * index];
        result[1] = ch[2 * index + 1];
        result[2] = L'\0';
    }
    return result;
}

Rgb getBlockColor(SHORT index, Scene *scene)
{
    const Rgb rgbs[] = {
        {0},
        {0, 0, 0, 0, 158, 96},    // 常春藤绿
        {0, 0, 0, 255, 185, 15},  // 向日葵黄
        {0, 0, 0, 0, 195, 179},   // 石青
        {0, 0, 0, 210, 105, 30},  // 可可棕
        {0, 0, 0, 65, 105, 225},  // 皇家蓝
        {0, 0, 0, 250, 128, 114}, // 鲑鱼红
        {0, 0, 0, 147, 112, 219}, // 藤萝紫
        {0, 0, 0, 255, 0, 0}};    // 炸弹红
    if (scene->mode == 0)
        return rgbs[index];
    return Rgb(rgbs[index].br, rgbs[index].bg, rgbs[index].bb);
}

Object *newBlock(SHORT x, SHORT y)
{
    SHORT index = rand() % NUM_TETROMINOS;
    Object *block = newObject(x, y, BLOCK_WIDTH, BLOCK_HEIGHT);
    if (rand() % 10 == 0)
        block->status = B_BOMB;
    for (int i = 0; i < BLOCK_HEIGHT; i++)
        for (int j = 0; j < BLOCK_WIDTH; j++)
            block->data[i][j] = TETROMINOS[index][i][j];

    // 随机旋转
    for (int i = 0; i < rand() % 4; i++)
        rotateObject(block);

    return block;
}

// 获取最靠左点X
SHORT getBlockX(Object *block)
{
    for (int j = 0; j < block->w; j++)
        for (int i = 0; i < block->h; i++)
            if (block->data[i][j])
                return j + block->x;
    return -1;
}

bool isCollided(Object *block, Object *map, int offsetX, int offsetY)
{
    for (int i = 0; i < block->h; i++)
    {
        for (int j = 0; j < block->w; j++)
        {
            if (block->data[i][j])
            {
                if (block->x + j + offsetX <= 0 || block->x + j + offsetX >= map->w - 1 || block->y + i + offsetY >= map->h - 1)
                    return true;
                // 允许穿过顶部掉落
                if (block->y + i + offsetY <= 0)
                    continue;
                if (map->data[block->y + i + offsetY][block->x + j + offsetX])
                    return true;
            }
        }
    }
    return false;
}

COORD canRotate(Object *block, Object *map, CHAR **rotatedData)
{
    Object *rotatedBlock = newObject(block->x, block->y, block->h, block->w);
    rotatedBlock->data = rotatedData;
    if (!isCollided(rotatedBlock, map, 0, 0))
        return (COORD){0, 0};

    // 墙踢（Wall Kick）：当旋转后碰到墙壁或障碍物时，系统会尝试微调方块位置以允许旋转
    for (short offset = 1; offset <= rotatedBlock->w / 2; offset++)
    {
        if (!isCollided(rotatedBlock, map, -offset, 0))
            return (COORD){-offset, 0};
        if (!isCollided(rotatedBlock, map, offset, 0))
            return (COORD){offset, 0};
    }
    for (short offset = 1; offset <= rotatedBlock->h / 2; offset++)
    {
        if (!isCollided(rotatedBlock, map, 0, -offset))
            return (COORD){0, -offset};
        if (!isCollided(rotatedBlock, map, 0, offset))
            return (COORD){0, offset};
    }
    return (COORD){-1, -1};
}

void rotateBlock(Object *block, Object *map)
{
    SHORT w = block->w;
    SHORT h = block->h;
    CHAR **rotatedData = (CHAR **)malloc(w * sizeof(CHAR *));
    for (int i = 0; i < w; i++)
    {
        rotatedData[i] = (CHAR *)malloc(h * sizeof(CHAR));
    }

    rotate90Clockwise(h, w, block->data, rotatedData);

    COORD offset = canRotate(block, map, rotatedData);
    if (offset.X == -1 && offset.Y == -1)
    {
        // 不能旋转，释放内存
        for (int k = 0; k < w; k++)
            free(rotatedData[k]);
        free(rotatedData);
        return;
    }

    // 释放原数据
    for (int i = 0; i < h; i++)
        free(block->data[i]);
    free(block->data);

    block->data = rotatedData;
    block->w = h;
    block->h = w;
    block->x += offset.X;
    block->y += offset.Y;
    playSoundSeq(6, 600, 30, 0, 800, 30, 0);
}

DWORD WINAPI explodeBlock(LPVOID lpParam)
{
    ExplodeParams *params = (ExplodeParams *)lpParam;
    Object *block = params->block;
    Object *map = params->map;
    Object *frags = NULL;

    for (SHORT i = 0; i < block->h; i++)
    {
        for (SHORT j = 0; j < block->w; j++)
        {
            if (block->data[i][j] == 0)
                continue;
            for (SHORT r = -1; r <= 2; r++)
            {
                for (SHORT c = -2; c <= 2; c++)
                {
                    SHORT r0 = block->y + i + r;
                    SHORT c0 = block->x + j + c;
                    if (r0 <= 0 || r0 >= map->h - 1 || c0 <= 0 || c0 >= map->w - 1 || map->data[r0][c0] == 0)
                        continue;
                    Object *frag = newObject(c0, r0, 1, 1);
                    frag->data[0][0] = map->data[r0][c0];
                    frag->vx = c == 0 ? -1 : c;
                    frag->vy = -1;
                    if (frags)
                    {
                        if (frags->next != NULL)
                            frag->next = frags->next;
                        frags->next = frag;
                    }
                    else
                        frags = frag;
                    map->data[r0][c0] = 0;
                }
            }
        }
    }
    block->status = B_DOCKED;

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
            if (f->vx == 0 || f->x <= map->x || f->x >= map->x + map->w - 1 || f->y <= map->y || f->y >= map->y + map->h - 1 || map->data[f->y][f->x] > 0)
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
        Sleep(80);
    }

    return 0;
}

void updateBlock(Object *block, Object *map)
{
    static clock_t lastTime = 0;
    clock_t currentTime = clock();
    if (currentTime - lastTime > 500)
    {
        lastTime = currentTime;
        if (isCollided(block, map, 0, 1))
        {
            // 炸弹
            if (block->status == B_BOMB)
            {
                ExplodeParams *params = (ExplodeParams *)malloc(sizeof(SoundParams));
                params->block = block;
                params->map = map;
                HANDLE hThread = CreateThread(NULL, 0, explodeBlock, params, 0, NULL);
                if (hThread)
                    CloseHandle(hThread);
            }
            else
            {
                for (SHORT i = 0; i < block->h; i++)
                {
                    for (SHORT j = 0; j < block->w; j++)
                    {
                        if (block->data[i][j])
                        {
                            //  固定方块
                            if (block->y + i > 0 && block->y + i < map->h - 1 && block->x + j > 0 && block->x + j < map->w - 1)
                                map->data[block->y + i][block->x + j] = block->data[i][j];
                        }
                    }
                }
                playSoundSeq(3, 400, 50, 0);
                block->status = B_DOCKED;
                HANDLE hThread = CreateThread(NULL, 0, clearLines, map, 0, NULL);
                if (hThread)
                    CloseHandle(hThread);
                // 结束游戏
                if (block->y <= -block->h / 2)
                {
                    block->status = B_OVER;
                    playSoundSeq(6, 220, 300, 100, 175, 500, 0);
                }
            }
        }
        else
        {
            block->y++;
        }
    }
}

void setSceneBlock(Scene *scene, Object *block, Object *map, bool showShadow)
{
    static clock_t lastTime = 0;
    clock_t currentTime = clock();
    if (currentTime - lastTime > 200)
        lastTime = currentTime;

    if (showShadow)
    {
        // 显示预期下落位置投影
        SHORT currentY = block->y;
        while (!isCollided(block, map, 0, 1))
            block->y += 1;
        for (SHORT i = 0; i < block->h; i++)
        {
            for (SHORT j = 0; j < block->w; j++)
            {
                // 忽略场景外的部分
                if (i + block->y < 0 || i + block->y >= scene->h || j + block->x < 0 || j + block->x >= scene->w)
                    continue;

                if (block->data[i][j])
                    scene->buffer[i + block->y][j + block->x].ch = _wcsdup(L"  ");

                if (vtSupported)
                {
                    if (block->data[i][j])
                        scene->buffer[i + block->y][j + block->x].color.rgb = block->status != B_BOMB || currentTime - lastTime > 50 ? (Rgb){0, 0, 0, 50, 50, 50} : Rgbb(0, 0, 0);
                }
            }
        }
        block->y = currentY;
    }

    for (SHORT i = 0; i < block->h; i++)
    {
        for (SHORT j = 0; j < block->w; j++)
        {
            // 忽略场景外的部分
            if (i + block->y < 0 || i + block->y >= scene->h || j + block->x < 0 || j + block->x >= scene->w)
                continue;

            if (block->data[i][j])
                scene->buffer[i + block->y][j + block->x].ch = block->status != B_BOMB || currentTime - lastTime > 50 ? getBlockCh(block->status == B_BOMB ? 8 : block->data[i][j], scene) : _wcsdup(L"  ");

            if (vtSupported)
            {
                if (block->data[i][j])
                    scene->buffer[i + block->y][j + block->x].color.rgb = block->status != B_BOMB || currentTime - lastTime > 50 ? getBlockColor(block->status == B_BOMB ? 8 : block->data[i][j], scene) : Rgb(0, 0, 0);
            }
        }
    }
}
