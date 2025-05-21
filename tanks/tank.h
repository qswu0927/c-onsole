#ifndef TANK_H
#define TANK_H
#include <windows.h>
#include <stdbool.h>
#include "../core/util.h"
#include "../core/render.h"

#define TANK_HEIGHT 3
#define TANK_WIDTH 3

struct Shell;

typedef struct Tank
{
    SHORT id;
    SHORT x, y;
    SHORT w, h;
    CHAR **data;
    Direction dir;
    Direction setDir;
    // 移动间隔毫秒数，用于控制速度
    int interMoveMs;
    // 上次移动时的计时器计数值
    LONGLONG lastMoveQpc;
    SHORT maxShellCount;
    SHORT shellCount;
    // 装载每发炮弹用时毫秒数
    int interLoadShellMs;
    // 上次装载炮弹时的计时器计数值
    LONGLONG lastLoadShellQpc;
    SHORT maxHealth;
    SHORT health;
    int score;
    SHORT killerId;
    bool npc;
    bool focus;
} Tank;

Tank *newTank(SHORT id, Object *map, Scene *scene, bool npc);
void updateTank(Tank **tanks, SHORT tankCount, SHORT index, struct Shell **shellPtr, Object *map, Scene *scene);
void setSceneTank(Scene *scene, Tank *tank);
void freeTank(Tank **tankPtr);
void changeFocus(Tank **tanks, SHORT tankCount, SHORT *focusOn);
void changeFocusNear(Tank **tanks, SHORT tankCount, SHORT *focusOn);

#endif
