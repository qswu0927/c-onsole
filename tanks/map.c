#include "map.h"
#include <windows.h>
#include "../core/util.h"
#include "../core/console.h"
#include "tank.h"

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
            return _wcsdup(L"火");

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
        // case 11:
        //     return _wcsdup(L"🚀");
        // case 12:
        //     return _wcsdup(L"🔥");
        // case 13:
        //     return _wcsdup(L"🍄");

    case 30:
        return _wcsdup(L"🔥");
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

Rgb getRgb(Scene *scene, SHORT data, SHORT i, SHORT j)
{
    Rgb rgbs[] = {{0}, {0, 0, 0, 10, 10, 10}};

    if (data == 0)
        return rgbs[(i / (scene->camera.h / 2) + j / (scene->camera.w / 2)) % ARRAY_LEN(rgbs)];

    if (scene->mode & RM_BG)
    {
        switch (data)
        {
        case 1:
            return Rgbb(0, 0, 255);
        case 10:
            return Rgbb(255, 160, 20);
        case 30:
        case 31:
        case 32:
        case 33:
            return Rgbb(140, 140, 140);
        default:
            return Rgbb(40, 120, 0);
        }
    }
    if (scene->mode & RM_CH)
    {
        switch (data)
        {
        case 1:
            return (Rgb){160, 160, 160, 0, 0, 255};

        case 10:
            return Rgb(255, 160, 20);

        case 30:
        case 31:
        case 32:
        case 33:
            return Rgb(140, 140, 140);

        default:
            return Rgb(0, 255, 0);
        }
    }
    switch (data)
    {
    case 1:
        return (Rgb){100, 100, 100, 0, 0, 255};
    case 10:
        return Rgb(255, 165, 0);
    default:
        return Rgb(255, 165, 0);
    }
}

bool isTankHit(SHORT x0, SHORT y0, Tank *tank)
{
    if (tank->health <= 0)
        return false;

    for (SHORT y = 0; y < tank->h; y++)
    {
        for (SHORT x = 0; x < tank->w; x++)
        {
            if (tank->data[y][x] == 0)
                continue;
            if (tank->x + x == x0 && tank->y + y == y0)
                return true;
        }
    }
    return false;
}

void initMap(Object *map, Tank **tanks, SHORT tankCount, Scene *scene)
{
    for (SHORT y = 0; y < map->h; y++)
        for (SHORT x = 0; x < map->w; x++)
            map->data[y][x] = y == map->y || y == map->y + map->h - 1 || x == map->x || x == map->x + map->w - 1 ? 1 : 0;
    for (SHORT t = 0; t < 20; t++)
        updateMap(map, tanks, tankCount, scene, false);
}

void updateMap(Object *map, Tank **tanks, SHORT tankCount, Scene *scene, bool outCamera)
{
    // 随机生成障碍
    SHORT blockW = 3 + rand() % 10;
    SHORT blockH = 3 + rand() % 10;
    SHORT x0 = rand() % (map->w - blockW + 1);
    SHORT y0 = rand() % (map->h - blockH + 1);

    bool valid = true;
    for (SHORT y = y0 - blockH / 2; y < y0 + blockH * 3 / 2; y++)
    {
        for (SHORT x = x0 - blockW / 2; x <= x0 + blockW * 3 / 2; x++)
        {
            if (x < map->x || x >= map->x + map->w || y < map->y || y >= map->y + map->h)
                continue;

            if (outCamera && x >= scene->camera.x && x < scene->camera.x + scene->camera.w && y >= scene->camera.y && y < scene->camera.y + scene->camera.h)
            {
                valid = false;
                break;
            }

            if (map->data[y][x] > 0)
            {
                valid = false;
                break;
            }

            for (SHORT i = 0; i < tankCount; i++)
            {
                if (tanks && isTankHit(x, y, tanks[i]))
                {
                    valid = false;
                    break;
                }
            }
            if (!valid)
                break;
        }
        if (!valid)
            break;
    }

    if (valid)
    {
        for (SHORT y = y0; y < y0 + blockH; y++)
            for (SHORT x = x0; x < x0 + blockW; x++)
                map->data[y][x] = 1;
    }
}

void setSceneMap(Scene *scene, Object *map)
{
    for (SHORT i = 0; i < map->h; i++)
    {
        for (SHORT j = 0; j < map->w; j++)
        {
            scene->buffer[i + map->y][j + map->x].ch = getCh(scene, map->data[i][j]);

            scene->buffer[i + map->y][j + map->x].color.rgb = getRgb(scene, map->data[i][j], i, j);
        }
    }
}
