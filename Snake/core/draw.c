#include <windows.h>
#include <stdio.h>
#include "draw.h"

void drawCharCube(int size, char *ch)
{
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            printf("%s", ch);
        }
        printf("\n");
    }
    printf("\n");
}

void drawCharCubeWithColor(int size, char *ch, int color)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
    drawCharCube(size, ch);
    SetConsoleTextAttribute(hConsole, 7); // Reset to default color
}

void drawCharCubeWithColorAndBackground(int size, char *ch, int color, int backgroundColor)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color | (backgroundColor << 4));
    drawCharCube(size, ch);
    SetConsoleTextAttribute(hConsole, 7); // Reset to default color
}