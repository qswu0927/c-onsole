#ifndef BLOCK_H
#define BLOCK_H
#include <stdbool.h>
#include "../core/render.h"

#define BLOCK_HEIGHT 4
#define BLOCK_WIDTH 4

extern const SHORT NUM_TETROMINOS;

typedef enum
{
    B_NONE,
    B_DOCKED,
    B_OVER,
    B_BOMB
} BlockStatus;

typedef struct
{
    Object *block;
    Object *map;
} ExplodeParams;

wchar_t *getBlockCh(SHORT index, Scene *scene);
Rgb getBlockColor(SHORT index, Scene *scene);
Object *newBlock(SHORT x, SHORT y);
SHORT getBlockX(Object *block);
bool isCollided(Object *block, Object *map, SHORT offsetX, SHORT offsetY);
void rotateBlock(Object *block, Object *map);
DWORD WINAPI explodeBlock(LPVOID lpParam);
void updateBlock(Object *block, Object *map);
void setSceneBlock(Scene *scene, Object *block, Object *map, bool showShadow);

#endif
