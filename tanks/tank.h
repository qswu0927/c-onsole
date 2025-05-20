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
    SHORT x, y;
    SHORT w, h;
    CHAR **data;
    Direction dir;
    Direction setDir;
    // 更新间隔毫秒数，用于控制速度
    int interMs;
    // 上次更新时的计时器计数值
    LONGLONG lastQpc;
    SHORT shells;
    SHORT maxHealth;
    SHORT health;
    int score;
    struct Tank *killBy;
    bool npc;
    bool focus;
} Tank;

Tank *newTank(Object *map, Scene *scene, bool npc);
void updateTank(Tank **tanks, SHORT tankCount, SHORT index, struct Shell **shellPtr, Object *map, Scene *scene);
void setSceneTank(Scene *scene, Tank *tank);
void freeTank(Tank **tankPtr);
void changeFocus(Tank **tanks, SHORT tankCount, SHORT *focusOn);
void changeFocusNear(Tank **tanks, SHORT tankCount, SHORT *focusOn);

#endif
