#ifndef MUSIC_H
#define MUSIC_H

typedef void (*SoundCallback)(void *context);
typedef struct
{
    int *frequencies;       // 频率数组指针
    int *durations;         // 持续时间数组指针
    int count;              // 音效数量
    int pause;              // 停顿时间
    SoundCallback callback; // 回调函数指针
    void *callbackContext;  // 回调参数
} SoundParams;

void playSound(SoundParams *params);

#endif
