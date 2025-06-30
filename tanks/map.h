#ifndef MAP_H
#define MAP_H
#include "../core/render.h"

struct Tank;

wchar_t *getCh(Scene *scene, SHORT data);
Rgb getRgb(Scene *scene, SHORT data, SHORT i, SHORT j);
bool isTankHit(SHORT x0, SHORT y0, struct Tank *tank);
void initMap(Object *map, struct Tank **tanks, SHORT tankCount, Scene *scene);
void updateMap(Object *map, struct Tank **tanks, SHORT tankCount, Scene *scene, bool outCamera);
void setSceneMap(Scene *scene, Object *map);

#endif
