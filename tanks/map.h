#ifndef MAP_H
#define MAP_H
#include "../core/render.h"

wchar_t *getCh(Scene *scene, SHORT data);
void initMap(Object *map);
void setSceneMap(Scene *scene, Object *map);

#endif
