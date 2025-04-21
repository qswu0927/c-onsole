#include "console.h"

bool vtSupported = false;

bool getVtSupport(void)
{
#ifdef _WIN32
    DWORD mode = 0;
    return GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &mode) &&
           (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#else
    return true; // 非Windows默认支持
#endif
}

// 启用虚拟终端序列处理
int enableVirtualTerminalProcessing(void)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE)
        return -1;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hConsole, &dwMode))
        return -1;

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hConsole, dwMode))
        return -1;

    return 0;
}

// 禁用鼠标选取控制台文本内容
void disableConsoleTextSelection(void)
{
    HANDLE hConsole = GetStdHandle(STD_INPUT_HANDLE);
    DWORD consoleMode;

    GetConsoleMode(hConsole, &consoleMode);

    // 禁用快速编辑模式（鼠标选取）
    consoleMode &= ~ENABLE_QUICK_EDIT_MODE;
    // 禁用鼠标输入
    consoleMode &= ~ENABLE_MOUSE_INPUT;
    SetConsoleMode(hConsole, consoleMode);
}

// 禁止调整控制台窗口大小
void disableConsoleResize(void)
{
    HWND hWnd = GetConsoleWindow();
    LONG style = GetWindowLong(hWnd, GWL_STYLE);
    // 禁止调整窗口大小
    style &= ~WS_THICKFRAME;
    // 禁用最大化按钮
    style &= ~WS_MAXIMIZEBOX;
    // 移除滚动条
    style &= ~(WS_VSCROLL | WS_HSCROLL);
    SetWindowLong(hWnd, GWL_STYLE, style);
}

// 隐藏光标
void hideConsoleCursor(void)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

// 设置控制台字体（支持大小、粗细、字体名）
BOOL setConsoleFont(HANDLE hConsole, int size, const wchar_t *fontName)
{
    CONSOLE_FONT_INFOEX fontInfo = {0};
    fontInfo.cbSize = sizeof(fontInfo);
    fontInfo.nFont = 0;                                 // 字体索引
    fontInfo.dwFontSize.X = 0;                          // 自动计算宽度
    fontInfo.dwFontSize.Y = size;                       // 字体高度（如 16）
    fontInfo.FontFamily = FF_DONTCARE;                  // 字体族
    fontInfo.FontWeight = FW_NORMAL;                    // 粗细（FW_BOLD 加粗）
    wcscpy_s(fontInfo.FaceName, LF_FACESIZE, fontName); // 字体名（如 L"Consolas"）

    return SetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);
}

HANDLE setupConsole(SHORT w, SHORT h, wchar_t *title)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // 启用虚拟终端序列处理
    enableVirtualTerminalProcessing();
    vtSupported = getVtSupport();

    // 设置控制台输出为 UTF-8 编码
    SetConsoleOutputCP(CP_UTF8);

    // 设置控制台标题
    SetConsoleTitleW(title);

    // 设置缓冲区大小
    SetConsoleScreenBufferSize(hConsole, (COORD){w, h});

    // 设置窗口大小（必须 <= 缓冲区大小）
    SetConsoleWindowInfo(hConsole, TRUE, &(SMALL_RECT){0, 0, w - 1, h - 1});

    // 设置控制台字体，其他推荐：|新宋体|黑体|MS Gothic
    setConsoleFont(hConsole, 18, L"黑体");

    disableConsoleResize();
    disableConsoleTextSelection();
    hideConsoleCursor();

    return hConsole;
}
