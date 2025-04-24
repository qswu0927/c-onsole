#ifndef BLOCK_H
#define BLOCK_H
#include <stdbool.h>
#include "../core/render.h"

Object *newBlock(void);
bool isCollided(Object *block, Object *map, int offsetX, int offsetY);
void rotateBlock(Object *block, Object *map);
void updateBlock(Object *block, Object *map);
void updateBlockToScene(Scene *scene, Object *block);

#endif
