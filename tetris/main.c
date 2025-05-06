#include "../core/util.h"
#include "../core/console.h"
#include "../core/render.h"
#include "../core/music.h"
#include "map.h"
#include "block.h"

#define CONFILE "config.dat"

typedef enum
{
    S_START,
    S_READY,
    S_PAUSED,
    S_OVER,
    S_RESTART,
    S_QUIT
} Status;

typedef struct
{
    RenderMode mode;
    int maxScore;
    bool mute;
    bool mouseInput;
} Config;
Config config = {.mode = RM_BG, .maxScore = 0, .mute = false, .mouseInput = false};

SHORT w = 12, h = 22;
COORD currentPos;
Status status = S_START;

Scene *scene = NULL;
Object *map = NULL;
Object *block = NULL;
Object *nextBlock = NULL;

FrameRate fr;

void handleInputEvent(INPUT_RECORD *record, WORD vkCode, DWORD mState)
{
    if (vkCode == 'A' || vkCode == VK_LEFT)
    {
        if (status == S_READY && !isCollided(block, map, -1, 0))
        {
            block->x -= 1;
            playSoundSeq(3, 800, 30, 0);
        }
    }
    else if (vkCode == 'D' || vkCode == VK_RIGHT)
    {
        if (status == S_READY && !isCollided(block, map, 1, 0))
        {
            block->x += 1;
            playSoundSeq(3, 800, 30, 0);
        }
    }
    else if (vkCode == 'W' || vkCode == VK_UP || mState & RIGHTMOST_BUTTON_PRESSED)
    {
        if (status == S_READY)
            rotateBlock(block, map);
    }
    else if (vkCode == 'S' || vkCode == VK_DOWN)
    {
        if (status == S_READY && !isCollided(block, map, 0, 1))
        {
            block->y += 1;
            playSoundSeq(3, 800, 30, 0);
        }
    }
    else if (vkCode == VK_SPACE || mState & FROM_LEFT_1ST_BUTTON_PRESSED)
    {
        while (status == S_READY && !isCollided(block, map, 0, 1))
            block->y += 1;
    }
    else if (vkCode == 'M')
    {
        scene->mode = (RenderMode)((scene->mode + 1) % RM_CNT);
        config.mode = scene->mode;
    }
    else if (vkCode == 'P')
    {
        if (block->status == 2)
            status = S_RESTART;
        else
            status = status == S_READY ? S_PAUSED : S_READY;
    }
    else if (vkCode == 'K')
        config.mute = !config.mute;
    else if (vkCode == 'L')
        config.mouseInput = !config.mouseInput;
    else if (vkCode == VK_ESCAPE)
        status = S_QUIT;

    MOUSE_EVENT_RECORD mer = record->Event.MouseEvent;
    // 鼠标移动检测
    if (mer.dwEventFlags & MOUSE_MOVED && mer.dwButtonState == 0)
    {
        // 与上次坐标对比，显著移动才触发
        static COORD lastPos = {0};
        currentPos = mer.dwMousePosition;
        currentPos.X /= 2;
        if (currentPos.X != lastPos.X && currentPos.X < map->x + map->w)
        {
            lastPos = currentPos;
            int offset = currentPos.X > getBlockX(block) ? 1 : -1;
            while (status == S_READY && !isCollided(block, map, offset, 0))
            {
                if (currentPos.X == getBlockX(block))
                    break;
                block->x += offset;
                playSoundSeq(3, 800, 30, 0);
            }
        }
    }
}

void setMessage(void)
{
    SHORT row = nextBlock->y + nextBlock->h + 1;
    SHORT col = w + 1;
    setSceneStr(scene, col, 0, Rgb(120, 120, 120), "下一个:");
    setSceneStr(scene, col, row++, Rgb(155, 155, 0), "得  分: %5d", score);
    setSceneStr(scene, col, row++, Rgb(150, 100, 0), "最高分: %5d", config.maxScore);
    setSceneStr(scene, col, row++, (Rgb){0, 0, 0, 100, 100, 100}, "输出模式: %d，按M切换", scene->mode);
    setSceneStr(scene, col, row++, Rgb(120, 120, 120), "移动: ASD|方向键");
    setSceneStr(scene, col, row++, Rgb(120, 120, 120), "旋转: W|上键");
    setSceneStr(scene, col, row++, Rgb(120, 120, 120), "速降: 空格键");
    if (block->status == 2)
    {
        static SHORT count = 0;
        if (count++ > 10)
            count = 0;
        if (count > 5)
            setSceneStr(scene, w / 2 - 2, h / 3, (Rgb){255, 255, 255, 200, 50, 0}, "游戏结束");
        setSceneStr(scene, col, row++, Rgb(0, count > 5 ? 200 : 100, 0), "重新开始: P键");
    }
    else
        setSceneStr(scene, col, row++, Rgb(110, 110, 110), "暂停|继续: P键");
    setSceneStr(scene, col, row++, Rgb(110, 110, 110), "退出: Esc键");
    setSceneStr(scene, col, row++, Rgb(110, 110, 110), config.mute ? "打开音效: K键" : "关闭音效: K键");
    if (config.mouseInput)
    {
        setSceneStr(scene, col, row++, Rgb(110, 110, 110), "禁用鼠标控制: L键");
        // setSceneStr(scene, col, row++, Rgb(79, 79, 79), "鼠标: %d,%d", currentPos.X, currentPos.Y);
        setSceneStr(scene, col, row++, Rgb(110, 110, 110), "速降: 鼠标左击");
        setSceneStr(scene, col, row++, Rgb(110, 110, 110), "旋转: 鼠标右击");
        setSceneStr(scene, col, row++, Rgb(110, 110, 110), "移动: 鼠标移动");
    }
    else
    {
        setSceneStr(scene, col, row++, Rgb(110, 110, 110), "启用鼠标控制: L键");
    }
    setSceneStr(scene, col, row++, Rgb(79, 79, 79), "帧率: %5.2fFps", 1000.0 / fr.elapsedMs);
}

void quit(void)
{
    saveConfig(CONFILE, &config, sizeof(Config));
}

int main(void)
{
    loadConfig(CONFILE, &config, sizeof(Config));
    mute = &config.mute;
    mouseInput = &config.mouseInput;
    setupConsole((w + 12) * 2, h, L"Tetris");
    setupClose(quit);
    scene = newScene(w + 12, h);
    scene->mode = config.mode;
START:
    status = S_START;
    if (map)
        freeObject(&map);
    map = newObject(0, 0, w, h);
    HANDLE hInitMapThread = CreateThread(NULL, 0, initMap, map, 0, NULL);
    if (block)
        freeObject(&block);
    if (nextBlock)
        freeObject(&nextBlock);
    nextBlock = newBlock(0, 0);
    fr = newFrameRate(12);
    // 开场音效
    playSoundSeq(7 * 3, 659, 100, 20,
                 494, 100, 20,
                 523, 100, 20,
                 587, 100, 20,
                 523, 100, 20,
                 494, 100, 20,
                 440, 100, 20);
    while (status != S_QUIT)
    {
        frameStart(&fr);

        if (block)
        {
            if (block->status == B_OVER)
                status = S_OVER;
            else if (block->status == B_DOCKED)
                freeObject(&block);
        }

        if (!block)
        {
            block = nextBlock;
            block->x = map->w / 2 - block->w / 2;
            block->y = -block->h / 2;
            nextBlock = newBlock(w + 1, 1);
        }

        getInput(handleInputEvent);
        if (status == S_RESTART)
            goto START;

        DWORD initMapExitCode;
        if (hInitMapThread && GetExitCodeThread(hInitMapThread, &initMapExitCode))
        {
            if (initMapExitCode != STILL_ACTIVE)
            {
                CloseHandle(hInitMapThread);
                hInitMapThread = NULL;
                status = S_READY;
            }
        }

        if (status == S_READY)
        {
            updateMap(map);
            updateBlock(block, map);
        }

        setSceneMap(scene, map);
        if (status != S_START)
        {
            setSceneBlock(scene, block, map, true);
            setSceneBlock(scene, nextBlock, map, false);
            // 渲染爆炸碎片
            Object *tmp = map->next;
            while (tmp)
            {
                setSceneBlock(scene, tmp, map, false);
                tmp = tmp->next;
            }
        }

        if (score > config.maxScore)
            config.maxScore = score;

        setMessage();
        renderScene(scene);
        frameEnd(&fr);
    }

    return 0;
}
