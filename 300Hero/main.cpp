#include <iostream>
#include "300config.h"
#include "hearthstoneconfig.h"
#include <Windows.h>

GameConfig myGame = config300Hero;

GameModeConfig* currentMode = nullptr;

std::string selectedModeName = "grail";

std::string currentState;

// 声明你要调用的游戏运行函数，比如 Run300Game()
void Run300Game();

void RunHearthStone();

int main()
{
    std::cout << "程序启动\n";

    // 注册 *** 键为全局热键（全局热键能后台监听）
    if (!RegisterHotKey(NULL, 1, MOD_CONTROL | MOD_SHIFT, 'C') || !RegisterHotKey(NULL, 2, MOD_CONTROL | MOD_SHIFT, VK_F12))
    {
        std::cerr << "注册热键失败！" << std::endl;
        return 1;
    }

    std::cout << "热键：[Ctrl+Shift+C] [Ctrl+Shift+F12]" << std::endl;

    while (true)
    {
        MSG msg = { 0 };
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_HOTKEY)
            {
                if (msg.wParam == 1) // Ctrl+Shift+C
                {
                    std::cout << "[Ctrl+Shift+C] 触发,暂停 6 秒..." << std::endl;
                    Sleep(6000);
                    std::cout << "恢复运行" << std::endl;
                }
                else if (msg.wParam == 2) // Ctrl+Shift+F12
                {
                    std::cout << "[Ctrl+Shift+F12] 触发,程序退出" << std::endl;
                    return 0;
                }
                
            }
        }


        if (myGame.exeName == config300Hero.exeName)
        {
            Run300Game();
        }
        else if (myGame.exeName == configHearthStone.exeName)
        {
            RunHearthStone();
        }
        else if (myGame.exeName == configHearthStone.exeName)
        {

        }
        Sleep(1000);
    }
    
    return 0;
}