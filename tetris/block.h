#ifndef BLOCK_H
#define BLOCK_H
#include <stdbool.h>
#include "../core/render.h"

RGBColor getBlockColor(SHORT index);
Object *newBlock(SHORT x, SHORT y);
bool isCollided(Object *block, Object *map, int offsetX, int offsetY);
void rotateBlock(Object *block, Object *map);
void updateBlock(Object *block, Object *map);
void updateBlockToScene(Scene *scene, Object *block, Object *map);
void updateNextBlockToScene(Scene *scene, Object *block);

#endif
