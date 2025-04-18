#ifndef UTIL_H
#define UTIL_H

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

/**
 * 方向
 */
typedef enum
{
    DIR_NONE,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

/**
 * 通用节点
 */
typedef struct Node
{
    SHORT x, y;
    SHORT data;
    struct Node *next;
} Node;

int listLen(Node *head);
char *listToStr(Node *head, char *con);

#endif
