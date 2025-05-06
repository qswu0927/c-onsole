#ifndef MAP_H
#define MAP_H
#include "../core/render.h"

extern int score;

DWORD WINAPI initMap(LPVOID lpParam);
void setSceneMap(Scene *scene, Object *map);
DWORD WINAPI clearLines(LPVOID lpParam);

#endif
