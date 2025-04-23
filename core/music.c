#include "music.h"
#include <windows.h>

DWORD WINAPI play8BitSound(LPVOID lpParam)
{
    // 从参数中获取音效数据
    SoundParams *params = (SoundParams *)lpParam;

    for (int i = 0; i < params->count;)
    {
        int f = params->data[i++];
        int d = params->data[i++];
        int p = params->data[i++];
        Beep(f, d);
        Sleep(p);
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
        // 不关心线程句柄
        CloseHandle(hThread);
    }
}
