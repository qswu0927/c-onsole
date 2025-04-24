#include "block.h"
#include <time.h>
#include "../core/util.h"
#include "../core/console.h"

#define BLOCK_HEIGHT 4
#define BLOCK_WIDTH 4
#define TETROMINO_COUNT 7

// 方块形状定义
const int TETROMINOS[TETROMINO_COUNT][BLOCK_HEIGHT][BLOCK_WIDTH] = {
    // I
    {{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    // O
    {{0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}},
    // T
    {{0, 0, 0, 0}, {0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}},
    // L
    {{0, 0, 0, 0}, {0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}},
    // J
    {{0, 0, 0, 0}, {1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}},
    // S
    {{0, 0, 0, 0}, {0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}},
    // Z
    {{0, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}}};

Object *newBlock(void)
{
    int index = rand() % TETROMINO_COUNT;
    // index = 0;
    Object *block = newObject(0, 0, BLOCK_WIDTH, BLOCK_HEIGHT);
    for (int i = 0; i < BLOCK_HEIGHT; i++)
    {
        for (int j = 0; j < BLOCK_WIDTH; j++)
        {
            block->data[i][j] = TETROMINOS[index][i][j];
        }
    }
    return block;
}

bool isCollided(Object *block, Object *map, int offsetX, int offsetY)
{
    for (int i = 0; i < block->h; i++)
    {
        for (int j = 0; j < block->w; j++)
        {
            if (block->data[i][j])
            {
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
    for (short offset = 1; offset < rotatedBlock->w; offset++)
    {
        if (!isCollided(rotatedBlock, map, -offset, 0))
            return (COORD){-offset, 0};
        if (!isCollided(rotatedBlock, map, offset, 0))
            return (COORD){offset, 0};
    }
    for (short offset = 1; offset < rotatedBlock->h; offset++)
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
}

void updateBlock(Object *block, Object *map)
{
    static clock_t lastFallTime = 0;
    clock_t currentTime = clock();
    if ((currentTime - lastFallTime) > 500)
    {
        lastFallTime = currentTime;
        if (!isCollided(block, map, 0, 1))
            block->y++;
    }
}

RGBColor getBlockRgb(CHAR data)
{
    if (data)
    {
        return (RGBColor){0, 0, 0, 255, 0, 0};
    }
    else
    {
        return (RGBColor){0, 0, 0, 29, 29, 29};
    }
}

void updateBlockToScene(Scene *scene, Object *block)
{
    for (SHORT i = 0; i < block->h; i++)
    {
        for (SHORT j = 0; j < block->w; j++)
        {
            // 忽略场景外的部分
            if (i + block->y < 0 || i + block->y >= scene->h || j + block->x < 0 || j + block->x >= scene->w)
                continue;

            scene->buffer[i + block->y][j + block->x].ch = L"  ";

            if (vtSupported)
            {
                scene->buffer[i + block->y][j + block->x].color.rgb = getBlockRgb(block->data[i][j]);
            }
        }
    }
}
