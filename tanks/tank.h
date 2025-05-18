#ifndef TANK_H
#define TANK_H
#include <windows.h>
#include <stdbool.h>
#include "../core/util.h"
#include "../core/render.h"

#define TANK_HEIGHT 3
#define TANK_WIDTH 3
#define TANK_MAX_HEALTH 99
#define SPEED_0 0
#define SPEED_L 6
#define SPEED_H 15

struct Shell;

typedef struct Tank
{
    SHORT x, y;
    SHORT w, h;
    CHAR **data;
    Direction dir;
    Direction setDir;
    SHORT speed;
    SHORT tick;
    SHORT shells;
    int health;
    int score;
    struct Tank *killBy;
    bool npc;
    bool focus;
} Tank;

Tank *newTank(Object *map, Scene *scene, bool npc);
void updateTank(Tank **tanks, SHORT tankCount, SHORT index, struct Shell *shell, Object *map, Scene *scene);
void setSceneTank(Scene *scene, Tank *tank);
void freeTank(Tank **tankPtr);
void changeFocus(Tank **tanks, SHORT tankCount, SHORT *focusOn);
void changeFocusNear(Tank **tanks, SHORT tankCount, SHORT *focusOn);

#endif
