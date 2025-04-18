#include <windows.h>
#include "window.h"

/**
 * 启用虚拟终端序列处理
 */
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

// 禁用鼠标滚轮（拦截 WM_MOUSEWHEEL）
LRESULT CALLBACK consoleWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_MOUSEWHEEL || uMsg == WM_MOUSEHWHEEL)
    {
        return 0; // 直接丢弃滚轮消息
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void disableMouseWheelScroll(void)
{
    HWND hWnd = GetConsoleWindow();
    if (hWnd)
    {
        SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)consoleWindowProc);
    }
}

// 禁用鼠标选取控制台文本内容
void disableConsoleTextSelection(void)
{
    HANDLE hConsole = GetStdHandle(STD_INPUT_HANDLE);
    DWORD consoleMode;

    // 获取当前控制台模式
    GetConsoleMode(hConsole, &consoleMode);

    // 禁用快速编辑模式和鼠标输入
    consoleMode &= ~ENABLE_QUICK_EDIT_MODE; // 禁用快速编辑模式（鼠标选取）
    consoleMode &= ~ENABLE_MOUSE_INPUT;     // 禁用鼠标输入
    SetConsoleMode(hConsole, consoleMode);
}

void disableConsoleScrolling(void)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // 1. 获取当前窗口大小
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    SHORT width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    SHORT height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    // 2. 设置缓冲区大小 = 窗口大小（彻底禁用滚动）
    COORD bufferSize = {width, height};
    SetConsoleScreenBufferSize(hConsole, bufferSize);

    // 3. 锁定窗口位置（防止用户手动滚动）
    SMALL_RECT windowRect = {0, 0, width - 1, height - 1};
    SetConsoleWindowInfo(hConsole, TRUE, &windowRect);

    // 4. 禁用鼠标滚轮和键盘滚动
    HWND hWnd = GetConsoleWindow();
    if (hWnd)
    {
        // 移除滚动条样式
        LONG style = GetWindowLong(hWnd, GWL_STYLE);
        style &= ~(WS_VSCROLL | WS_HSCROLL); // 移除滚动条
        style &= ~WS_THICKFRAME;             // 禁止调整窗口大小
        style &= ~WS_MAXIMIZEBOX;            // 禁用最大化按钮
        SetWindowLong(hWnd, GWL_STYLE, style);

        // 强制重绘窗口
        // SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

        // 拦截滚轮消息
        SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)DefWindowProc);
    }
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

    enableVirtualTerminalProcessing();

    // 设置控制台输出为 UTF-8 编码
    SetConsoleOutputCP(CP_UTF8);
    // 设置控制台输入为 UTF-8 编码
    // 某些系统上可能会导致问题，可以根据需要选择性使用
    // SetConsoleCP(CP_UTF8);

    // 设置控制台标题
    SetConsoleTitleW(title);

    // 设置控制台字体，其他推荐：|新宋体|黑体|MS Gothic
    setConsoleFont(hConsole, 18, L"黑体");

    hideConsoleCursor();

    disableConsoleTextSelection();

    // // 设置缓冲区大小（必须 >= 窗口大小）
    // SetConsoleScreenBufferSize(hConsole, (COORD){w, h});

    // // 设置窗口大小（必须 <= 缓冲区大小）
    // SetConsoleWindowInfo(hConsole, TRUE, &(SMALL_RECT){0, 0, w - 1, h - 1});

    return hConsole; // 返回控制台句柄

    // DisableConsoleScrolling();
}
