#ifndef BLOCK_H
#define BLOCK_H
#include <stdbool.h>
#include "../core/render.h"

wchar_t *getBlockCh(SHORT index, Scene *scene);
Rgb getBlockColor(SHORT index, Scene *scene);
Object *newBlock(SHORT x, SHORT y);
SHORT getBlockX(Object *block);
bool isCollided(Object *block, Object *map, int offsetX, int offsetY);
void rotateBlock(Object *block, Object *map);
void updateBlock(Object *block, Object *map);
void setSceneBlock(Scene *scene, Object *block, Object *map, bool showShadow);

#endif
