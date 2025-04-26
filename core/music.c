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

    // 释放内存
    if (params->needFree)
    {
        free(params->data);
        free(params);
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

// 可变参数版本的playSound函数
void playSound2(int count, ...)
{
    // 分配足够的内存存储音效数据
    int *data = (int *)malloc(count * sizeof(int));
    if (!data)
        return;

    // 获取可变参数
    va_list args;
    va_start(args, count);

    for (int i = 0; i < count; i++)
    {
        data[i] = va_arg(args, int);
    }
    va_end(args);

    // 创建音效参数
    SoundParams *params = (SoundParams *)malloc(sizeof(SoundParams));
    if (!params)
    {
        free(data);
        return;
    }

    params->data = data;
    params->count = count / 3; // 每组3个参数(频率,时长,暂停)
    params->needFree = true;
    params->callback = NULL;
    params->callbackContext = NULL;

    // 创建线程播放音效，线程结束时自动释放内存
    HANDLE hThread = CreateThread(
        NULL,
        0,
        play8BitSound,
        params,
        0,
        NULL);

    if (hThread)
    {
        CloseHandle(hThread);
    }
    else
    {
        // 创建线程失败，手动释放内存
        free(data);
        free(params);
    }
}
