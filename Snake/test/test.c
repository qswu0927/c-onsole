#include <windows.h>
#include <stdio.h>
#include "../core/window.h"
#include "../core/window.h"
#include "test.h"

#define WIDTH 20
#define HEIGHT 10

void testWriteConsoleOutput()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD bufferSize = {2, 1};
    COORD bufferCoord = {0, 0};
    CHAR_INFO *charInfo = (CHAR_INFO *)malloc(10 * sizeof(CHAR_INFO));
    SMALL_RECT writeRegion = {0, 0, 1, 0};

    charInfo[0].Char.AsciiChar = 'A';
    charInfo[0].Attributes = FOREGROUND_RED;
    charInfo[1].Char.AsciiChar = 'B';
    charInfo[1].Attributes = FOREGROUND_GREEN;

    WriteConsoleOutput(hConsole, charInfo, bufferSize, bufferCoord, &writeRegion);
    getchar();
}

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
            // buffer[i][j] = L'◼';

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

void testChessboard()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    // SetConsoleOutputCP(CP_UTF16);

    // 3. 初始化棋盘（使用全角字符）
    CHAR_INFO chessBoard[8][8];
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            chessBoard[y][x].Char.UnicodeChar = (x + y) % 2 ? L'我' : L'你';
            chessBoard[y][x].Attributes = (x + y) % 2 ? 0x0F : 0xF0;
        }
    }

    // 4. 调整缓冲区大小
    COORD bufferSize = {16, 8};
    SetConsoleScreenBufferSize(hConsole, bufferSize);

    // 5. 输出到控制台
    SMALL_RECT area = {0, 0, 15, 7};
    WriteConsoleOutputW(
        hConsole,
        (CHAR_INFO *)chessBoard,
        (COORD){8, 16},
        (COORD){0, 0},
        &area);
    getchar();
}

void testUtf8WithColor()
{
    SetConsoleOutputCP(CP_UTF8);

    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD){10, 5});
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
    printf("红色中文😄");

    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD){15, 8});
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BACKGROUND_GREEN | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    printf("绿色背景白字中文😄🐍😄");

    // 重置为默认颜色
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    getchar();
}

void testEmoji()
{
    SetConsoleOutputCP(CP_UTF8);
    printf("中文测试 ✅\n");
    printf("Emoji 测试: 😊🌟\n");
    printf("Unicode 转义: \\U0001F970 -> \U0001F970\n"); // 🥰
    getchar();
}

void testWriteConsoleW()
{
    // HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    // SetConsoleOutputCP(CP_UTF8);
    // DWORD written;
    // wchar_t text[] = L"Hello, 世界●◼😄和🐍";

    // size_t len = wcslen(text);

    // WriteConsoleOutputCharacterW(
    //     hConsole, text, wcslen(text), (COORD){0, 0}, &written);

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleOutputCP(CP_UTF8);
    DWORD written;
    wchar_t text[] = L"Hello, 世界●◼😄\n和🐍";

    // 使用 WriteConsoleW 替代 WriteConsoleOutputCharacterW
    WriteConsoleW(hConsole, text, wcslen(text), &written, NULL);
    getchar();
}