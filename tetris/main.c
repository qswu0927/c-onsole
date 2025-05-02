#include "../core/util.h"
#include "../core/console.h"
#include "../core/render.h"
#include "../core/music.h"
#include "map.h"
#include "block.h"

typedef struct
{
    RenderMode mode;
    int maxScore;
    bool mouseOp;
} Config;

bool isPaused = false;
bool isQuit = false;
Config config = {.mode = RM_BG, .maxScore = 0, .mouseOp = false};

void quit(void)
{
    saveConfig("config.dat", &config, sizeof(Config));
}

int main(void)
{
    loadConfig("config.dat", &config, sizeof(Config));
    SHORT w = 12, h = 22;
    setupConsole((w + 12) * 2, h, L"Tetris");
    setupClose(quit);

    Scene *scene = newScene(w + 12, h);
START:
    isPaused = false;
    Object *map = newObject(0, 0, w, h);
    updateMap(map);
    Object *block = NULL;
    Object *nextBlock = newBlock(0, 0);
    FrameRate fr = newFrameRate(12);
    // 开场音效
    playSound2(7 * 3, 659, 100, 20,
               494, 100, 20,
               523, 100, 20,
               587, 100, 20,
               523, 100, 20,
               494, 100, 20,
               440, 100, 20);
    while (!isQuit)
    {
        frameStart(&fr);

        if (block)
        {
            if (block->status == 2)
                isPaused = true;
            else if (block->status == 1)
                freeObject(&block);
        }

        if (!block)
        {
            block = nextBlock;
            block->x = map->w / 2 - block->w / 2;
            block->y = -block->h / 2;
            nextBlock = newBlock(w + 1, 1);
        }

        {
            int k = getKb();
            if (k == 27)
                isQuit = true;
            else if (k == 'P')
            {
                if (block->status == 2)
                {
                    freeObject(&block);
                    freeObject(&nextBlock);
                    freeObject(&map);
                    goto START;
                }
                else
                    isPaused = !isPaused;
            }
            else if (k == 'M')
            {
                scene->mode = (RenderMode)((scene->mode + 1) % RM_CNT);
                config.mode = scene->mode;
            }
            else if (k == 'A' || k == 0xE04B)
            {
                if (!isPaused && !isCollided(block, map, -1, 0))
                {
                    block->x -= 1;
                    playSound2(3, 800, 30, 0);
                }
            }
            else if (k == 'D' || k == 0xE04D)
            {
                if (!isPaused && !isCollided(block, map, 1, 0))
                {
                    block->x += 1;
                    playSound2(3, 800, 30, 0);
                }
            }
            else if (k == 'W' || k == 0xE048)
            {
                if (!isPaused)
                    rotateBlock(block, map);
            }
            else if (k == 'S' || k == 0xE050)
            {
                if (!isPaused && !isCollided(block, map, 0, 1))
                {
                    block->y += 1;
                    playSound2(3, 800, 30, 0);
                }
            }
            else if (k == ' ')
            {
                while (!isPaused && !isCollided(block, map, 0, 1))
                    block->y += 1;
            }
        }

        INPUT_RECORD irInBuf[128];
        DWORD cNumRead;
        COORD currentPos;
        // 非阻塞检查事件
        PeekConsoleInput(hStdIn, irInBuf, 128, &cNumRead);
        if (cNumRead > 0)
        {
            ReadConsoleInput(hStdIn, irInBuf, 128, &cNumRead);

            for (DWORD i = 0; i < cNumRead; i++)
            {
                if (irInBuf[i].EventType == MOUSE_EVENT)
                {
                    MOUSE_EVENT_RECORD mer = irInBuf[i].Event.MouseEvent;

                    // 鼠标左键按下
                    if (mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
                    {
                        while (!isPaused && !isCollided(block, map, 0, 1))
                            block->y += 1;
                    }

                    if (mer.dwButtonState & RIGHTMOST_BUTTON_PRESSED)
                    {
                        if (!isPaused)
                            rotateBlock(block, map);
                    }
                    // 鼠标移动检测
                    if (mer.dwEventFlags & MOUSE_MOVED)
                    {
                        currentPos = mer.dwMousePosition;
                        int offset = currentPos.X / 2 > getBlockX(block) ? 1 : -1;
                        while (!isPaused && !isCollided(block, map, offset, 0))
                        {
                            if (currentPos.X / 2 == getBlockX(block))
                                break;
                            block->x += offset;
                            playSound2(3, 800, 30, 0);
                        }
                    }
                }
            }
        }

        if (!isPaused)
        {
            updateBlock(block, map);
        }

        setSceneMap(scene, map);
        setSceneBlock(scene, block, map);
        setSceneNextBlock(scene, nextBlock);

        if (score > config.maxScore)
            config.maxScore = score;

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
            setSceneStr(scene, w / 2 - 3, h / 3, count > 5 ? Rgb(200, 50, 0) : (Rgb){200, 200, 200, 200, 50, 0}, "😂游戏结束😂");
            setSceneStr(scene, col, row++, Rgb(0, count > 5 ? 200 : 100, 0), "重新开始: P键");
        }
        else
            setSceneStr(scene, col, row++, Rgb(110, 110, 110), "暂停|继续: P键");
        setSceneStr(scene, col, row++, Rgb(110, 110, 110), "退出: Esc键");
        setSceneStr(scene, col, row++, Rgb(79, 79, 79), "帧率: %5.2fFps", 1000.0 / fr.elapsedMs);
        if (config.mouseOp)
        {
            setSceneStr(scene, col, row++, Rgb(79, 79, 79), "鼠标:%d,%d", currentPos.X, currentPos.Y);
        }
        else
        {
            setSceneStr(scene, col, row++, Rgb(110, 110, 110), "启用鼠标控制: L键");
        }

        renderScene(scene);

        frameEnd(&fr);
    }

    return 0;
}
