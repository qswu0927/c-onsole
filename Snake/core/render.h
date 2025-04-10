#ifndef RENDER_H
#define RENDER_H

// "●◼◻⊙★○"

typedef struct
{
    SHORT x, y;
    SHORT w, h;
    char **buffer;
} Object;

typedef struct
{
    SHORT w, h;
    char **buffer;
} Scene;

Scene *newScene(SHORT w, SHORT h);
void addToScene(Scene *scene, Object *obj);
void renderScene(Scene *obj);
void freeScene(Scene *scene);
Object *newObject(SHORT x, SHORT y, SHORT w, SHORT h);
void freeObject(Object *obj);

#endif