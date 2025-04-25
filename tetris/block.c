#include "block.h"
#include <time.h>
#include "../core/util.h"
#include "../core/console.h"
#include "map.h"

#define BLOCK_HEIGHT 4
#define BLOCK_WIDTH 4
#define TETROMINO_COUNT 7

// 方块形状定义
const int TETROMINOS[TETROMINO_COUNT][BLOCK_HEIGHT][BLOCK_WIDTH] = {
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

RGBColor getBlockColor(SHORT index)
{
    const RGBColor rgbs[] = {
        {0},
        {0, 0, 0, 0, 255, 255},
        {0, 0, 0, 255, 255, 0},
        {0, 0, 0, 255, 0, 255},
        {0, 0, 0, 0, 255, 0},
        {0, 0, 0, 0, 0, 255},
        {0, 0, 0, 255, 0, 0},
        {0, 0, 0, 255, 128, 0}};
    return rgbs[index];
}

Object *newBlock(void)
{
    SHORT index = rand() % TETROMINO_COUNT;
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
                if (block->x + j + offsetX < 0 || block->x + j + offsetX >= map->w || block->y + i + offsetY < 0 || block->y + i + offsetY >= map->h)
                    return true; // 越界
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
    Beep(600, 5);
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
                        map->data[block->y + i][block->x + j] = block->data[i][j];
                    }
                }
            }
            block->status = 1; // 设置状态为已固定
            HANDLE hThread = CreateThread(
                NULL,       // 默认安全属性
                0,          // 默认堆栈大小
                clearLines, // 线程函数
                map,        // 传入参数
                0,          // 默认创建标志
                NULL        // 不需要线程ID
            );

            if (hThread)
            {
                // 不关心线程句柄
                CloseHandle(hThread);
            }
        }
        else
        {
            block->y++;
        }
    }
}

void updateBlockToScene(Scene *scene, Object *block, Object *map)
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

            scene->buffer[i + block->y][j + block->x].ch = L"  ";

            if (vtSupported)
            {
                if (block->data[i][j])
                    scene->buffer[i + block->y][j + block->x].color.rgb = (RGBColor){0, 0, 0, 40, 40, 40};
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

            scene->buffer[i + block->y][j + block->x].ch = L"  ";

            if (vtSupported)
            {
                if (block->data[i][j])
                    scene->buffer[i + block->y][j + block->x].color.rgb = getBlockColor(block->data[i][j]);
            }
        }
    }
}
