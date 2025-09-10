#include <iostream>
#include "300config.h"
#include "hearthstoneconfig.h"
#include <Windows.h>

GameConfig myGame = config300Hero;

GameModeConfig* currentMode = nullptr;

std::string selectedModeName = "grail";

std::string currentState;

// ������Ҫ���õ���Ϸ���к��������� Run300Game()
void Run300Game();

void RunHearthStone();

int main()
{
    std::cout << "��������\n";

    // ע�� *** ��Ϊȫ���ȼ���ȫ���ȼ��ܺ�̨������
    if (!RegisterHotKey(NULL, 1, MOD_CONTROL | MOD_SHIFT, 'C') || !RegisterHotKey(NULL, 2, MOD_CONTROL | MOD_SHIFT, VK_F12))
    {
        std::cerr << "ע���ȼ�ʧ�ܣ�" << std::endl;
        return 1;
    }

    std::cout << "�ȼ���[Ctrl+Shift+C] [Ctrl+Shift+F12]" << std::endl;

    while (true)
    {
        MSG msg = { 0 };
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_HOTKEY)
            {
                if (msg.wParam == 1) // Ctrl+Shift+C
                {
                    std::cout << "[Ctrl+Shift+C] ����,��ͣ 6 ��..." << std::endl;
                    Sleep(6000);
                    std::cout << "�ָ�����" << std::endl;
                }
                else if (msg.wParam == 2) // Ctrl+Shift+F12
                {
                    std::cout << "[Ctrl+Shift+F12] ����,�����˳�" << std::endl;
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