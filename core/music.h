#ifndef MUSIC_H
#define MUSIC_H
#include <stdbool.h>

typedef void (*SoundCallback)(void *context);
typedef struct
{
    int *data;              // 音频数据，按【频率，持续时长，停顿时长】3个参数排列
    int count;              // 音效数量
    SoundCallback callback; // 回调函数指针
    void *callbackContext;  // 回调参数
    bool needFree;          // 是否需要释放data内存
} SoundParams;

void playSound(SoundParams *params);
void playSound2(int count, ...);

#endif
