#ifndef CONSOLE_H
#define CONSOLE_H
#include <windows.h>
#include <stdbool.h>

// 是否支持虚拟终端序列
extern bool vtSupported;

void hideConsoleCursor(void);
BOOL setConsoleFont(HANDLE hConsole, int size, const wchar_t *fontName);
HANDLE setupConsole(SHORT w, SHORT h, wchar_t *title);

#endif
