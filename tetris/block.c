#include "block.h"
#include <time.h>
#include "../core/util.h"
#include "../core/console.h"
#include "../core/music.h"
#include "map.h"

#define BLOCK_HEIGHT 4
#define BLOCK_WIDTH 4

// 基本方块
const int TETROMINOS[][BLOCK_HEIGHT][BLOCK_WIDTH] = {
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
    {{0, 0, 0, 0}, {7, 7, 0, 0}, {0, 7, 7, 0}, {0, 0, 0, 0}},
    // 炸弹
    {{0, 0, 0, 0}, {8, 0, 8, 0}, {0, 8, 0, 0}, {8, 0, 8, 0}}};

// 炸弹方块
const int BOMB[][BLOCK_HEIGHT][BLOCK_WIDTH] = {
    {{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{0, 0, 0, 0}, {0, 2, 2, 0}, {0, 2, 2, 0}, {0, 0, 0, 0}}};

wchar_t *getBlockCh(SHORT index)
{
    if (index == 8)
        return _wcsdup(L"💣");
    return _wcsdup(L"  ");
}

Rgb getBlockColor(SHORT index)
{
    const Rgb rgbs[] = {
        {0},
        {0, 0, 0, 0, 158, 96},    // 常春藤绿
        {0, 0, 0, 255, 185, 15},  // 向日葵黄
        {0, 0, 0, 0, 195, 179},   // 石青
        {0, 0, 0, 210, 105, 30},  // 可可棕
        {0, 0, 0, 65, 105, 225},  // 皇家蓝
        {0, 0, 0, 250, 128, 114}, // 鲑鱼红
        {0, 0, 0, 147, 112, 219}, // 鲑鱼红
        {255, 0, 0, 255, 0, 0}};  // 炸弹
    return rgbs[index];
}

Object *newBlock(SHORT x, SHORT y)
{
    SHORT index = rand() % (sizeof(TETROMINOS) / sizeof(TETROMINOS[0]));
    Object *block = newObject(x, y, BLOCK_WIDTH, BLOCK_HEIGHT);
    for (int i = 0; i < BLOCK_HEIGHT; i++)
    {
        for (int j = 0; j < BLOCK_WIDTH; j++)
        {
            block->data[i][j] = TETROMINOS[index][i][j];
        }
    }
    // 随机旋转
    for (int i = 0; i < rand() % 4; i++)
        rotateObject(block);

    return block;
}

// 获取最靠左点X
SHORT getBlockX(Object *block)
{
    for (int j = 0; j < block->w; j++)
    {
        for (int i = 0; i < block->h; i++)
        {
            if (block->data[i][j])
            {
                return j + block->x;
            }
        }
    }
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
    playSound2(6, 600, 30, 0, 800, 30, 0);
}

void updateBlock(Object *block, Object *map)
{
    static clock_t lastFallTime = 0;
    clock_t currentTime = clock();
    if ((currentTime - lastFallTime) > 500)
    {
        lastFallTime = currentTime;
        if (isCollided(block, map, 0, 1))
        {
            // 固定方块
            for (int i = 0; i < block->h; i++)
            {
                for (int j = 0; j < block->w; j++)
                {
                    if (block->data[i][j])
                    {
                        // 忽略场景外的部分
                        if (block->y + i >= 0 && block->y + i < map->h && block->x + j >= 0 && block->x + j < map->w)
                            map->data[block->y + i][block->x + j] = block->data[i][j];
                    }
                }
            }
            playSound2(3, 400, 50, 0);
            block->status = 1;
            CreateThread(NULL, 0, clearLines, map, 0, NULL);
            // 结束游戏
            if (block->y <= 0)
            {
                block->status = 2;
                playSound2(6, 220, 300, 100, 175, 500, 0);
            }
        }
        else
        {
            block->y++;
        }
    }
}

void setSceneBlock(Scene *scene, Object *block, Object *map)
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
                    scene->buffer[i + block->y][j + block->x].color.rgb = (Rgb){0, 0, 0, 50, 50, 50};
            }
        }
    }
    block->y = currentY;

    for (SHORT i = 0; i < block->h; i++)
    {
        for (SHORT j = 0; j < block->w; j++)
        {
            // 忽略场景外的部分
            if (i + block->y < 0 || i + block->y >= scene->h || j + block->x < 0 || j + block->x >= scene->w)
                continue;

            if (block->data[i][j])
                scene->buffer[i + block->y][j + block->x].ch = getBlockCh(block->data[i][j]);

            if (vtSupported)
            {
                if (block->data[i][j])
                    scene->buffer[i + block->y][j + block->x].color.rgb = getBlockColor(block->data[i][j]);
            }
        }
    }
}

void setSceneNextBlock(Scene *scene, Object *block)
{
    for (SHORT i = 0; i < block->h; i++)
    {
        for (SHORT j = 0; j < block->w; j++)
        {
            // 忽略场景外的部分
            if (i + block->y < 0 || i + block->y >= scene->h || j + block->x < 0 || j + block->x >= scene->w)
                continue;

            scene->buffer[i + block->y][j + block->x].ch = _wcsdup(L"  ");

            if (vtSupported)
            {
                if (block->data[i][j])
                    scene->buffer[i + block->y][j + block->x].color.rgb = getBlockColor(block->data[i][j]);
            }
        }
    }
}
