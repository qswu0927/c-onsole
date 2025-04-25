#ifndef MAP_H
#define MAP_H
#include "../core/render.h"

void updateMap(Object *map);
void updateMapToScene(Scene *scene, Object *map);
DWORD WINAPI clearLines(LPVOID lpParam);

#endif
