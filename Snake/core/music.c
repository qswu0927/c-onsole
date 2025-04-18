#include <windows.h>
#include "music.h"

DWORD WINAPI play8BitSound(LPVOID lpParam)
{
    // 从参数中获取音效数据
    SoundParams *params = (SoundParams *)lpParam;

    for (int i = 0; i < params->count; i++)
    {
        Beep(params->frequencies[i], params->durations[i]);
        Sleep(params->pause);
    }

    // 播放完成后调用回调
    if (params->callback)
    {
        params->callback(params->callbackContext);
    }

    return 0;
}

void playSound(SoundParams *params)
{
    HANDLE hThread = CreateThread(
        NULL,          // 默认安全属性
        0,             // 默认堆栈大小
        play8BitSound, // 线程函数
        params,        // 传入参数
        0,             // 默认创建标志
        NULL           // 不需要线程ID
    );

    if (hThread)
    {
        CloseHandle(hThread); // 我们不关心线程句柄
    }
}
