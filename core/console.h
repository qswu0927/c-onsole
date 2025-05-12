#ifndef CONSOLE_H
#define CONSOLE_H
#include <windows.h>
#include <stdbool.h>

typedef void (*QuitFunc)(void);

extern HANDLE hStdOut;
extern HANDLE hStdIn;
extern HWND hWnd;
// 是否支持虚拟终端序列
extern bool vtSupported;
// 是否监听鼠标输入
extern bool *mouseInput;

void hideConsoleCursor(void);
BOOL setConsoleFont(HANDLE hStdOut, SHORT size, const wchar_t *fontName);
BOOL setConsoleFontSize(HANDLE hStdOut, SHORT size);
void setupConsole(SHORT w, SHORT h, SHORT fontSize, wchar_t *title);
void setupClose(QuitFunc quit);

#endif
