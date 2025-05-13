#include "map.h"
#include <windows.h>
#include "../core/util.h"
#include "../core/console.h"

void initMap(Object *map)
{
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
