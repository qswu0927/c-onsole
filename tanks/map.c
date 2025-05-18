#include "map.h"
#include <windows.h>
#include "../core/util.h"
#include "../core/console.h"

wchar_t *getCh(Scene *scene, SHORT data)
{
    if (scene->mode & RM_BG)
        return _wcsdup(L"  ");
    if (scene->mode & RM_CH)
    {
        switch (data)
        {
        case 0:
            return _wcsdup(L"  ");
        case 1:
            return _wcsdup(L"墙");

        case 10:
            return _wcsdup(L"鸟");
        case 11:
            return _wcsdup(L"蛙");
        case 12:
            return _wcsdup(L"鱼");
        case 13:
            return _wcsdup(L"鼠");

        case 30:
            return _wcsdup(L"炮");
        case 31:
            return _wcsdup(L"坦");
        case 32:
            return _wcsdup(L"克");
        case 33:
            return _wcsdup(L"士");

        default:
            return _wcsdup(L"哈");
        }
    }
    switch (data)
    {
    case 0:
        return _wcsdup(L"  ");
    case 1:
        return _wcsdup(L"🚧");

    case 10:
        return _wcsdup(L"🎇");
    case 11:
        return _wcsdup(L"🚀");
    case 12:
        return _wcsdup(L"🔥");
    case 13:
        return _wcsdup(L"🍄");

    case 30:
        return _wcsdup(L"🚩");
        // return _wcsdup(L"😄额👮‍♂️👷‍♂️👷‍♀️👩‍🚒👳‍♂️👩👶😲😳🙊发⚙🧱⚙🔋🚌🚖🚩🚍");
    case 31:
        return _wcsdup(L"🚖");
    case 32:
        return _wcsdup(L"👩");
    case 33:
        return _wcsdup(L"😲");

    default:
        return _wcsdup(L"😄");
    }
}

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
            scene->buffer[i + map->y][j + map->x].ch = getCh(scene, map->data[i][j]);

            scene->buffer[i + map->y][j + map->x].color.rgb = getMapRgb(scene, map->data[i][j], i, j);
        }
    }
}
