#include "../core/util.h"
#include "../core/console.h"
#include "../core/render.h"
#include "../core/music.h"
#include "map.h"
#include "shell.h"
#include "tank.h"

#define CONFILE "config.dat"

typedef enum
{
    S_START = 1,
    S_READY = 1 << 1,
    S_PAUSED = 1 << 2,
    S_OVER = 1 << 3,
    S_RESTART = 1 << 4,
    S_QUIT = 1 << 5,
    S_WIN = 1 << 6,
} Status;

typedef struct
{
    RenderMode mode;
    SHORT fontSize;
    bool mute;
    int maxScore;
} Config;
Config config = {.mode = RM_CH, .fontSize = 18, .mute = false, .maxScore = 0};

FrameRate fr;
SHORT w = 25, h = 25;
SHORT messageW = 15;
Status status = S_START;

Scene *scene = NULL;
Object *map = NULL;
Tank **tanks = NULL;
SHORT tankCount = 10;
SHORT aliveCount = 0;
SHORT focusOn = 0;
Shell *shell = NULL;

void handleInputEvent(InputEvent e)
{
    Direction dir = DIR_NONE;
    if (e.vkdCode == 'A' || e.vkdCode == VK_LEFT)
        dir = DIR_LEFT;
    else if (e.vkdCode == 'D' || e.vkdCode == VK_RIGHT)
        dir = DIR_RIGHT;
    else if (e.vkdCode == 'W' || e.vkdCode == VK_UP)
        dir = DIR_UP;
    else if (e.vkdCode == 'S' || e.vkdCode == VK_DOWN)
        dir = DIR_DOWN;
    else if (e.vkdCode == 'G' && !tanks[0]->alive)
        changeFocus(tanks, tankCount, &focusOn);
    else if (e.vkdCode == 'P')
    {
        if (tanks[0]->alive)
            status ^= S_READY | S_PAUSED;
        else
            status = S_RESTART;
    }
    else if (e.vkdCode == 'M')
        config.mode = changeSceneMode(scene);
    else if (e.vkdCode == VK_OEM_PLUS || e.vkdCode == VK_ADD)
        setConsoleFontSize(hStdOut, ++config.fontSize);
    else if (e.vkdCode == VK_OEM_MINUS || e.vkdCode == VK_SUBTRACT)
        setConsoleFontSize(hStdOut, --config.fontSize);
    else if (e.vkdCode == 'K')
        config.mute = !config.mute;
    else if (e.vkdCode == VK_ESCAPE)
        status = S_QUIT;

    if (status & S_READY && tanks[0]->alive && dir != DIR_NONE)
    {
        if (tanks[0]->setDir != dir)
            tanks[0]->setDir = dir;
        tanks[0]->speed = SPEED_H;
    }

    if (!e.kDown)
        tanks[0]->speed = SPEED_L;
}

void setCamera(void)
{
    static SHORT count = 0;
    if (tanks[focusOn]->alive == false)
        count++;
    // 镜头延迟切换
    if (count > 15)
    {
        changeFocusNear(tanks, tankCount, &focusOn);
        count = 0;
    }

    scene->camera.x = tanks[focusOn]->x - (scene->camera.w - messageW) / 2;
    scene->camera.y = tanks[focusOn]->y - scene->camera.h / 2;

    // 限位
    if (scene->camera.x + scene->camera.w > scene->w)
        scene->camera.x = scene->w - scene->camera.w;
    if (scene->camera.y + scene->camera.h > scene->h)
        scene->camera.y = scene->h - scene->camera.h;
    if (scene->camera.x < 0)
        scene->camera.x = 0;
    if (scene->camera.y < 0)
        scene->camera.y = 0;
}

void setMessage(void)
{
    static SHORT clearH = 0;
    clearSceneP(scene, scene->camera.x + scene->camera.w - messageW, scene->camera.y, messageW, clearH);

    SHORT row = scene->camera.y;
    SHORT col = scene->camera.x + scene->camera.w - messageW;

    static SHORT count = 0;
    if (count++ > 10)
        count = 0;
    setSceneStr(scene, col, row++, Rgb(120, 120, 0), "得  分: %5d", tanks[0]->score);
    setSceneStr(scene, col, row++, Rgb(150, 100, 0), "最高分: %5d", config.maxScore);
    setSceneStr(scene, col, row++, Rgb(120, 120, 120), "玩家: %d/%d", aliveCount, tankCount);
    if (status & S_WIN)
        setSceneStr(scene, col, row++, Rgb(200, count > 5 ? 200 : 255, 0), "恭喜，您已胜出😄");
    else if (!tanks[0]->alive)
    {
        setSceneStr(scene, col, row++, Rgb(180, 50, 0), "哎呀，挑战失败😂");
        setSceneStr(scene, col, row++, Rgb(150, 150, count > 5 ? 150 : 255), "正在观战%d号，按G切换", focusOn);
    }
    setSceneStr(scene, col, row++, Rgb(120, 120, 120), "方向控制: WASD|方向键");
    setSceneStr(scene, col, row++, Rgb(120, 120, 120), "加速: 长按上述键");
    if (tanks[0]->alive == false)
    {
        setSceneStr(scene, col, row++, Rgb(0, count > 5 ? 200 : 100, 0), "重新开始: P键");
    }
    else
        setSceneStr(scene, col, row++, Rgb(100, 100, 100), "暂停|继续: P键");
    setSceneStr(scene, col, row++, Rgb(100, 100, 100), "退出: Esc键");
    setSceneStr(scene, col, row++, Rgb(0, 120, 120), "输出模式: %d，按M切换", scene->mode);
    setSceneStr(scene, col, row++, Rgb(120, 120, 120), "字体大小: %d，按+-调整", config.fontSize);
    setSceneStr(scene, col, row++, Rgb(100, 100, 100), config.mute ? "打开音效: K键" : "关闭音效: K键");
    setSceneStr(scene, col, row++, Rgb(80, 80, 80), "帧率: %5.2fFps", 1000.0 / fr.elapsedMs);

    clearH = row - scene->camera.y;
}

void quit(void)
{
    saveConfig(CONFILE, &config, sizeof(Config));
}

int main(void)
{
    loadConfig(CONFILE, &config, sizeof(Config));
    mute = &config.mute;
    setupConsole((w + messageW) * 2, h, config.fontSize, L"坦克大战");
    setupClose(quit);

    scene = newScene(w + messageW + 20, h + 20);
    scene->camera.w = w + messageW;
    scene->camera.h = h;
    scene->mode = config.mode;
    map = newObject(0, 0, scene->w - messageW, scene->h);

START:
    status = S_READY;
    initMap(map);
    if (tanks)
    {
        for (SHORT i = 0; i < tankCount; i++)
            freeTank(&tanks[i]);
        free(tanks);
    }
    tanks = (Tank **)malloc(sizeof(Tank *) * tankCount);
    for (SHORT i = 0; i < tankCount; i++)
        tanks[i] = NULL;
    tanks[0] = newTank(map, scene, false);
    focusOn = 0;

    if (shell)
        freeShell(&shell);
    shell = newShell(map->w * map->h / 100);

    // 开场音乐
    playSoundSeq(9, 300, 50, 20, 400, 100, 20, 300, 50, 20);

    fr = newFrameRate(12);
    while (status != S_QUIT)
    {
        frameStart(&fr);

        getInput(handleInputEvent);
        if (status == S_RESTART)
            goto START;

        if (status & S_READY)
        {
            for (SHORT i = 0; i < tankCount; i++)
            {
                updateTank(tanks, tankCount, i, shell, map, scene);
            }
            updateShell(shell, tanks, tankCount, map);
        }

        setCamera();
        setSceneMap(scene, map);
        setSceneShell(scene, shell);
        for (SHORT i = 0; i < tankCount; i++)
            setSceneTank(scene, tanks[i]);

        setMessage();
        renderScene(scene);

        if (tanks[0]->score > config.maxScore)
            config.maxScore = tanks[0]->score;

        aliveCount = getAliveCount(tanks, tankCount);
        if ((status & S_WIN) == 0 && aliveCount == 1 && tanks[0]->alive)
        {
            status |= S_WIN;
            // 胜利音乐
            playSoundSeq(12,
                         523, 200, 50, // C5 (Do)
                         587, 200, 50, // D5 (Re)
                         659, 200, 50, // E5 (Mi)
                         784, 400, 0);
        }

        frameEnd(&fr);
    }

    return 0;
}
