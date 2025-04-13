#ifndef WINDOW_H
#define WINDOW_H

void hideConsoleCursor();
BOOL setConsoleFont(HANDLE hConsole, int size, const wchar_t *fontName);
HANDLE setupConsole(SHORT w, SHORT h, wchar_t *title);

#endif