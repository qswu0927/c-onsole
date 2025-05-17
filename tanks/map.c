#include "map.h"
#include <windows.h>
#include "../core/util.h"
#include "../core/console.h"

void initMap(Object *map)
{
    for (SHORT y = 0; y < map->h; y++)
        for (SHORT x = 0; x < map->w; x++)
            map->data[y][x] = 1;
    // 随机生成一些空白区域
    for (SHORT t = 0; t < 30; t++)
    {
        SHORT x0 = rand() % map->w;
        SHORT y0 = rand() % map->h;
        for (SHORT y = y0 - map->h / 8; y <= y0 + map->h / 8; y++)
        {
            for (SHORT x = x0 - map->w / 8; x <= x0 + map->w / 8; x++)
            {
                if (y <= 0 || y >= map->h - 1 || x <= 0 || x >= map->w - 1)
                    continue;
                map->data[y][x] = 0;
            }
        }
    }
}

wchar_t *getMapCh(RenderMode mode, CHAR data)
{
    if (data == 0)
        return _wcsdup(L"  ");
    if (mode & RM_BG)
        return _wcsdup(L"  ");
    if (mode & RM_CH)
        return _wcsdup(L"墙");
    return _wcsdup(L"🚧");
}

Rgb getMapRgb(Scene *scene, CHAR data, SHORT i, SHORT j)
{
    Rgb rgbs[] = {{0}, {0, 0, 0, 10, 10, 10}};

    if (data == 0)
        return rgbs[(i / (scene->camera.h / 2) + j / (scene->camera.w / 2)) % ARRAY_LEN(rgbs)];

    if (scene->mode & RM_BG)
        return Rgbb(0, 0, 255);

    return (Rgb){100, 100, 100, 0, 0, 255};
}

void setSceneMap(Scene *scene, Object *map)
{
    for (SHORT i = 0; i < map->h; i++)
    {
        for (SHORT j = 0; j < map->w; j++)
        {
            scene->buffer[i + map->y][j + map->x].ch = getMapCh(scene->mode, map->data[i][j]);

            scene->buffer[i + map->y][j + map->x].color.rgb = getMapRgb(scene, map->data[i][j], i, j);
        }
    }
}
