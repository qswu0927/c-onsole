#ifndef TANK_H
#define TANK_H
#include <windows.h>
#include <stdbool.h>
#include "../core/util.h"
#include "../core/render.h"

#define TANK_HEIGHT 3
#define TANK_WIDTH 3
#define SPEED_L 6
#define SPEED_H 15

struct Shell;

typedef struct Tank
{
    SHORT x, y;
    SHORT w, h;
    CHAR status;
    CHAR **data;
    Direction dir;
    Direction setDir;
    SHORT speed;
    SHORT tick;
    SHORT shells;
    bool alive;
    int score;
    bool npc;
    bool focus;
} Tank;

Tank *newTank(Object *map, Scene *scene, bool npc);
void updateTank(Tank **tanks, SHORT tankCount, SHORT index, struct Shell *shell, Object *map, Scene *scene);
void setSceneTank(Scene *scene, Tank *tank);
SHORT getAliveCount(Tank **tanks, SHORT tankCount);
void freeTank(Tank **tankPtr);
void changeFocus(Tank **tanks, SHORT tankCount, SHORT *focusOn);
void changeFocusNear(Tank **tanks, SHORT tankCount, SHORT *focusOn);

#endif
