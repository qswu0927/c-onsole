#include <windows.h>
#include <stdio.h>
#include "core/draw.h"

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    printf("Hello, ♦World!\n");
    // drawCharCube(10, "●");
    // drawCharCube(10, "◼");
    // drawCharCube(10, "◻");
    // drawCharCube(10, "⊙");
    // drawCharCubeWithColor(10, "★", 0x0C);
    // drawCharCube(10, "○");
    drawCharCubeWithColorAndBackground(10, "★", 0x0C, 0x0A);
}