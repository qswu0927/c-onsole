#include <windows.h>
#include <stdio.h>
#include "render.h"

void render(Object *obj)
{
    SHORT wr = 2;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD bufferSize = {wr * obj->w, obj->h};
    COORD bufferCoord = {0, 0};
    SMALL_RECT writeRegion = {0, 0, wr * obj->w - 1, obj->h - 1};
    CHAR_INFO *charInfo = (CHAR_INFO *)malloc(wr * obj->w * obj->h * sizeof(CHAR_INFO));

    for (int i = 0; i < obj->h; i++)
    {
        for (int j = 0; j < obj->w; j++)
        {
            for (int k = 0; k < wr; k++)
            {
                int index = wr * (i * obj->w + j) + k;
                charInfo[index].Char.AsciiChar = ' ';
                charInfo[index].Attributes = obj->buffer[i][j] == '#'   ? BACKGROUND_BLUE
                                             : obj->buffer[i][j] == ' ' ? FOREGROUND_RED
                                                                        : BACKGROUND_GREEN;
            }
        }
    }
    WriteConsoleOutput(hConsole, charInfo, bufferSize, bufferCoord, &writeRegion);
    free(charInfo);
}

void freeObject(Object *obj)
{
    for (int i = 0; i < obj->h; i++)
    {
        free(obj->buffer[i]);
    }
    free(obj->buffer);
}