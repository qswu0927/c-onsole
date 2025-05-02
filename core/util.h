#ifndef UTIL_H
#define UTIL_H
#include <stdio.h>
#include <windows.h>

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

/**
 * 方向
 */
typedef enum
{
    DIR_UP,
    DIR_LEFT,
    DIR_DOWN,
    DIR_RIGHT,
    DIR_NONE
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
FILE *safeFopen(const char *filename, const char *mode);
void loadConfig(const char *filename, void *config, size_t size);
void saveConfig(const char *filename, void *config, size_t size);
void shuffle(int arr[], int n);
void rotate90Clockwise(int rows, int cols, CHAR **mat, CHAR **result);

#endif
