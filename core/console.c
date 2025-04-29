#include "console.h"
#include <stdio.h>

HANDLE hConsole = NULL;
HWND hWnd = NULL;
bool vtSupported = false;

bool getVtSupport(void)
{
#ifdef _WIN32
    DWORD mode = 0;
    return GetConsoleMode(hConsole, &mode) &&
           (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#else
    return true; // 非Windows默认支持
#endif
}

// 启用虚拟终端序列处理
int enableVirtualTerminalProcessing(void)
{
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

// 置顶控制台窗口
void setConsoleWindowTopmost(void)
{
    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

// 窗口居中
void setConsoleWindowCenter(void)
{
    // 获取屏幕分辨率
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // 获取控制台窗口当前大小
    RECT rectConsole;
    GetWindowRect(hWnd, &rectConsole);
    int consoleWidth = rectConsole.right - rectConsole.left;
    int consoleHeight = rectConsole.bottom - rectConsole.top;

    // 计算居中位置
    int posX = (screenWidth - consoleWidth) / 2;
    int posY = (screenHeight - consoleHeight) / 2;

    // 设置窗口位置
    SetWindowPos(
        hWnd,
        NULL,
        posX, posY,
        0, 0,
        SWP_NOSIZE | SWP_NOZORDER);
}

void setupConsole(SHORT w, SHORT h, wchar_t *title)
{
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    hWnd = GetConsoleWindow();

    // 启用虚拟终端序列处理
    enableVirtualTerminalProcessing();
    vtSupported = getVtSupport();

    // 设置控制台输出为 UTF-8 编码
    SetConsoleOutputCP(CP_UTF8);

    // 设置控制台标题
    SetConsoleTitleW(title);

    // 设置控制台字体，其他推荐：|新宋体|黑体|MS Gothic
    setConsoleFont(hConsole, 18, L"黑体");

    // 获取当前窗口大小
    // CONSOLE_SCREEN_BUFFER_INFO csbi;
    // GetConsoleScreenBufferInfo(hConsole, &csbi);
    // SHORT width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    // SHORT height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    // 1. 获取最大可能尺寸
    COORD maxSize = GetLargestConsoleWindowSize(hConsole);
    // 2. 检查请求的尺寸是否合理
    if (w < 12 || w > maxSize.X || h < 1 || h > maxSize.Y)
    {
        printf("请求的尺寸 %dx%d 超出允许范围\n", w, h);
        getchar();
    }
    // 3. 先设置缓冲区高度足够大
    if (!SetConsoleScreenBufferSize(hConsole, (COORD){maxSize.X, maxSize.Y}))
    {
        printf("无法设置缓冲区! 错误: %lu\n按回车键继续", GetLastError());
        getchar();
    }
    // 4. 缩小窗口到目标尺寸
    if (!SetConsoleWindowInfo(hConsole, TRUE, &(SMALL_RECT){0, 0, w - 1, h - 1}))
    {
        printf("无法缩小窗口! 错误: %lu\n按回车键继续", GetLastError());
        getchar();
    }
    // 5. 最后设置缓冲区大小
    if (!SetConsoleScreenBufferSize(hConsole, (COORD){w, h}))
    {
        printf("无法设置最终缓冲区! 错误: %lu\n按回车键继续", GetLastError());
        getchar();
    }
    // 6. 再次设置窗口大小，解决多一行一列的问题
    if (!SetConsoleWindowInfo(hConsole, TRUE, &(SMALL_RECT){0, 0, w - 1, h - 1}))
    {
        printf("无法设置最终窗口! 错误: %lu\n按回车键继续", GetLastError());
        getchar();
    }

// 调试模式下，将窗口置顶
#ifdef _DEBUG
    setConsoleWindowTopmost();
#endif
    // setConsoleWindowCenter();
    hideConsoleCursor();
    disableConsoleTextSelection();
    disableConsoleResize();
}
