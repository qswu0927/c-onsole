#ifndef CONSOLE_H
#define CONSOLE_H
#include <windows.h>
#include <stdbool.h>

extern HANDLE hStdOut;
extern HANDLE hStdIn;
extern HWND hWnd;
// 是否支持虚拟终端序列
extern bool vtSupported;

void hideConsoleCursor(void);
BOOL setConsoleFont(HANDLE hStdOut, int size, const wchar_t *fontName);
void setupConsole(SHORT w, SHORT h, wchar_t *title);
void freeConsole(void);

#endif
