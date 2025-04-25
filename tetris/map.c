#include "map.h"
#include "../core/console.h"
#include "block.h"

void updateMap(Object *map)
{
    for (int i = 0; i < map->h; i++)
    {
        for (int j = 0; j < map->w; j++)
        {
            if (i == 0 || i == map->h - 1 || j == 0 || j == map->w - 1)
                map->data[i][j] = 1;
            else
                map->data[i][j] = 0;
        }
    }
}

RGBColor getMapRgb(Object *map, SHORT i, SHORT j)
{
    if (i == 0 || i == map->h - 1 || j == 0 || j == map->w - 1)
        return (RGBColor){0, 0, 0, 0, 128, 128};

    if (map->data[i][j])
        return getBlockColor(map->data[i][j]);

    RGBColor rgbs[] = {{0}, {0, 0, 0, 12, 12, 12}};
    return rgbs[(i + j) % 2];
}

void updateMapToScene(Scene *scene, Object *map)
{
    for (SHORT i = 0; i < map->h; i++)
    {
        for (SHORT j = 0; j < map->w; j++)
        {
            scene->buffer[i + map->y][j + map->x].ch = L"  ";

            if (vtSupported)
            {
                scene->buffer[i + map->y][j + map->x].color.rgb = getMapRgb(map, i, j);
            }
            else
            {
                // todo
            }
        }
    }
}
