#include "map.h"
#include <windows.h>
#include "core/util.h"
#include "core/console.h"

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

wchar_t *getMapCh(RenderMode mode, CHAR data)
{
    if (data == 0)
        return L"  ";
    if (mode == RM_BG)
        return L"  ";
    if (mode == RM_CH)
        return L"墙";
    return L"🚧";
}
WORD getMapAttr(Scene *scene, CHAR data, SHORT i, SHORT j)
{
    // 深色模式
    WORD colors[] = {0, 0x80};
    // 浅色模式
    // WORD colors[] = {0x70, 0xf0};
    if (data == 0)
        return colors[(i / (scene->camera.h / 2) + j / (scene->camera.w / 2)) % ARRAY_LEN(colors)];
    if (scene->mode == RM_BG)
        return BACKGROUND_BLUE;
    return 0x01;
}

RGBColor getMapRgb(Scene *scene, CHAR data, SHORT i, SHORT j)
{
    // 深色模式
    RGBColor rgbs[] = {{0}, {0, 0, 0, 10, 10, 10}};
    // 浅色模式
    //  RGBColor rgbs[] = {{0, 0, 0, 255, 255, 255}, {0, 0, 0, 245, 245, 245}};

    if (data == 0)
        return rgbs[(i / (scene->camera.h / 2) + j / (scene->camera.w / 2)) % ARRAY_LEN(rgbs)];

    if (scene->mode == RM_BG)
        return (RGBColor){0, 0, 0, 0, 0, 255};

    return (RGBColor){0, 0, 255, 0, 0, 0};
}

void updateMapToScene(Scene *scene, Object *obj)
{
    for (SHORT i = 0; i < obj->h; i++)
    {
        for (SHORT j = 0; j < obj->w; j++)
        {
            scene->buffer[i + obj->y][j + obj->x].ch = getMapCh(scene->mode, obj->data[i][j]);

            if (vtSupported)
            {
                scene->buffer[i + obj->y][j + obj->x].color.rgb = getMapRgb(scene, obj->data[i][j], i, j);
            }
            else
            {
                scene->buffer[i + obj->y][j + obj->x].color.attr = getMapAttr(scene, obj->data[i][j], i, j);
            }
        }
    }
}
