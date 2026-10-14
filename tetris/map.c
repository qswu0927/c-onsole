#include "map.h"
#include "../core/util.h"
#include "../core/console.h"
#include "../core/music.h"
#include "block.h"

int score = 0;

DWORD WINAPI initMap(LPVOID lpParam)
{
    Object *map = (Object *)lpParam;
    for (SHORT i = 0; i < map->h; i++)
    {
        for (SHORT j = 0; j < map->w; j++)
        {
            if (i == 0 || i == map->h - 1 || j == 0 || j == map->w - 1)
                map->data[i][j] = 1;
            else
                map->data[i][j] = 0;
        }
    }
    // 过场动画，先由下往上填充，再由上往下清空
    for (SHORT i = map->h - 2; i >= 1; i--)
    {
        for (SHORT j = 1; j < map->w - 1; j++)
        {
            map->data[i][j] = rand() % NUM_TETROMINOS + 1;
        }
        Sleep(20);
    }
    for (SHORT i = 1; i < map->h - 1; i++)
    {
        for (SHORT j = 1; j < map->w - 1; j++)
        {
            if (i > map->h * 3 / 4)
                map->data[i][j] = rand() % 3 == 0 ? rand() % (NUM_TETROMINOS + 1) : 0;
            else
                map->data[i][j] = 0;
        }
        Sleep(20);
    }

    return 0;
}

// 添加底部障碍
void addMapLine(Object *map)
{
    for (SHORT i = 1; i < map->h - 2; i++)
    {
        for (SHORT j = 1; j < map->w - 1; j++)
        {
            map->data[i][j] = map->data[i + 1][j];
        }
    }
    for (SHORT j = 1; j < map->w - 1; j++)
    {
        map->data[map->h - 2][j] = rand() % 3 == 0 ? rand() % (NUM_TETROMINOS + 1) : 0;
    }
}

void updateMap(Object *map)
{
    if (rand() % 50 == 0)
        addMapLine(map);
}

wchar_t *getMapCh(Object *map, SHORT i, SHORT j, Scene *scene)
{
    if (i == map->y || i == map->y + map->h - 1 || j == map->x || j == map->x + map->w - 1)
        return _wcsdup(L"  ");

    return getBlockCh(map->data[i][j], scene);
}

Rgb getMapRgb(Object *map, SHORT i, SHORT j, Scene *scene)
{
    if (i == 0 || i == map->h - 1 || j == 0 || j == map->w - 1)
        return (Rgb){0, 0, 0, 0, 65, 65};

    Rgb rgbs[] = {{0}, {0, 0, 0, 12, 12, 12}};

    Rgb rgb = rgbs[(i + j) % 2];

    if (map->data[i][j])
        orColor(&rgb, getBlockColor(map->data[i][j], scene));

    return rgb;
}

void setSceneMap(Scene *scene, Object *map)
{
    for (SHORT i = 0; i < map->h; i++)
    {
        for (SHORT j = 0; j < map->w; j++)
        {
            scene->buffer[i + map->y][j + map->x].ch = getMapCh(map, i, j, scene);

            orColor(&scene->buffer[i + map->y][j + map->x].color.rgb, getMapRgb(map, i, j, scene));
        }
    }
}

DWORD WINAPI clearLines(LPVOID lpParam)
{
    Object *map = (Object *)lpParam;

    // 考虑连续消除多行的情况
    SHORT fullLineBegin = -1;
    SHORT fullLineEnd = -1;
    for (SHORT y = map->h - 2; y >= 1; y--)
    {
        bool full = true;
        for (SHORT x = 1; x < map->w - 1; x++)
        {
            if (!map->data[y][x])
            {
                full = false;
                break;
            }
        }

        if (full)
        {
            if (fullLineBegin == -1)
                fullLineBegin = y;
            fullLineEnd = y;
        }
        else
        {
            if (fullLineBegin == -1)
                continue;

            playSoundSeq(12, 523, 80, 0, 587, 80, 0, 659, 80, 0, 684 + (fullLineBegin - fullLineEnd) * 30, 150, 0);
            score += (fullLineBegin - fullLineEnd + 1) * (fullLineBegin - fullLineEnd + 1);

            // 闪烁效果
            // 保存闪烁点原本数据
            CHAR **flashData = (CHAR **)malloc((fullLineBegin - fullLineEnd + 1) * sizeof(CHAR *));
            for (SHORT yf = fullLineBegin; yf >= fullLineEnd; yf--)
            {
                flashData[fullLineBegin - yf] = (CHAR *)malloc(map->w * sizeof(CHAR));
                for (SHORT x = 1; x < map->w - 1; x++)
                    flashData[fullLineBegin - yf][x] = map->data[yf][x];
            }

            for (SHORT flashTime = 0; flashTime < 2; flashTime++)
            {
                for (SHORT yf = fullLineBegin; yf >= fullLineEnd; yf--)
                    for (SHORT x = 1; x < map->w - 1; x++)
                        map->data[yf][x] = 0;
                Sleep(100);
                for (SHORT yf = fullLineBegin; yf >= fullLineEnd; yf--)
                    for (SHORT x = 1; x < map->w - 1; x++)
                        map->data[yf][x] = flashData[fullLineBegin - yf][x];
                Sleep(100);
            }

            // 释放闪烁数据内存
            for (SHORT yf = fullLineBegin; yf >= fullLineEnd; yf--)
                free(flashData[fullLineBegin - yf]);
            free(flashData);

            // 下移所有行
            for (SHORT ny = fullLineBegin; ny > 1 + (fullLineBegin - fullLineEnd); ny--)
                for (SHORT x = 1; x < map->w - 1; x++)
                    map->data[ny][x] = map->data[ny - (fullLineBegin - fullLineEnd + 1)][x];

            // 清空顶部行
            for (SHORT ny = 1; ny <= fullLineBegin - fullLineEnd + 1; ny++)
                for (SHORT x = 1; x < map->w - 1; x++)
                    map->data[ny][x] = 0;

            // 重新检查
            y = fullLineBegin;
            fullLineBegin = -1;
        }
    }
    return 0;
}
