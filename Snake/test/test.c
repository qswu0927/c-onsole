#include <windows.h>
#include <stdio.h>
#include "../core/window.h"
#include "test.h"

#define WIDTH 20
#define HEIGHT 10

void testBufferOutput()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleOutputCP(CP_UTF8);
    hideConsoleCursor();

    COORD bufferSize = {WIDTH, HEIGHT};
    COORD bufferCoord = {0, 0};
    SMALL_RECT writeRegion = {0, 0, WIDTH - 1, HEIGHT - 1};
    CHAR_INFO buffer[WIDTH * HEIGHT] = {0};

    for (int i = 0; i < WIDTH; i++)
    {
        for (int j = 0; j < HEIGHT; j++)
        {
            buffer[i + j * WIDTH].Char.AsciiChar = 'A' + (i + j) % 26;
            // 不支持多字节
            // buffer[i + j * WIDTH].Char.UnicodeChar = L'我';
            // 使用背景色代替字符
            buffer[i + j * WIDTH].Char.AsciiChar = ' ';
            buffer[i + j * WIDTH].Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            if ((i + j) % 2 == 0)
                buffer[i + j * WIDTH].Attributes = BACKGROUND_RED;

            WriteConsoleOutput(hConsole, buffer, bufferSize, bufferCoord, &writeRegion);

            Sleep(10);
        }
    }
    getchar();
}

void testBufferOutputW()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleOutputCP(CP_UTF8);
    hideConsoleCursor();

    COORD bufferSize = {WIDTH, HEIGHT};
    COORD bufferCoord = {0, 0};
    SMALL_RECT writeRegion = {0, 0, WIDTH - 1, HEIGHT - 1};
    wchar_t buffer[HEIGHT][WIDTH] = {0}; // 使用宽字符缓冲区

    for (int i = 0; i < WIDTH; i++)
    {
        for (int j = 0; j < HEIGHT; j++)
        {
            buffer[i][j] = L'你';
            buffer[i][j] = L'◼';

            // 将缓冲区逐行写入控制台
            for (int k = 0; k < HEIGHT; k++)
            {
                DWORD charsWritten;
                COORD lineCoord = {0, k};
                WriteConsoleOutputCharacterW(hConsole, buffer[k], WIDTH, lineCoord, &charsWritten);
            }

            Sleep(10);
        }
    }
    getchar();
}
