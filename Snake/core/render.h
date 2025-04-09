#ifndef RENDER_H
#define RENDER_H

// "●◼◻⊙★○"

typedef struct
{
    SHORT x, y;
    SHORT w, h;
    char **buffer;
} Object;

void render(Object *obj);
void freeObject(Object *obj);

#endif