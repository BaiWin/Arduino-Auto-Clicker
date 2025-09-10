#include "utils.h"
#include "300config.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <random>

void RunHearthStone()
{
    // ��ʼ��ֻ��һ��
    static bool initialized = false;
    if (!initialized)
    {
        for (auto& mode : myGame.modes)
        {
            if (mode.modeName == selectedModeName)
            {
                std::cout << "��ǰģʽ��" << selectedModeName << std::endl;
                currentMode = &mode;
                currentState = mode.initialState;
                break;
            }
        }

        if (!currentMode)
        {
            std::cerr << "�Ҳ���ģʽ��" << selectedModeName << std::endl;
            return;
        }

        initialized = true;
    }

    SetProcessDPIAware();

    DWORD pid = FindPIDByExeName(std::wstring(myGame.exeName.begin(), myGame.exeName.end()));
    if (pid == 0) return;

    // Check1 �����б߿�
    HWND hwnd = FindMainWindowByPID(pid);
    if (!hwnd || hwnd != GetForegroundWindow()) return;

    // Check2 �����ޱ߿�
    HWND fgWnd = GetForegroundWindow();
    DWORD fgPID = 0;
    GetWindowThreadProcessId(fgWnd, &fgPID);
    if (fgPID != pid) { return; }

    cv::Mat screenshot = captureWindowFromScreen(hwnd);

    State* statePtr = nullptr;
    for (auto& state : currentMode->states)
    {
        if (state.name == currentState)
        {
            statePtr = &state;
            break;
        }
    }

    if (!statePtr)
    {
        std::cout << "δ�ҵ�״̬��" << currentState << std::endl;
        return;
    }

    // ��ӡ������꣬������
    //PrintMousePos();

    for (auto& action : statePtr->actions)   // �����¡
    {
        if (action.type == BehaviorType::ImageClick)
        {
            cv::Mat tmpl = cv::imread(action.imagePath, cv::IMREAD_UNCHANGED);

            if (tmpl.empty()) continue;

            cv::Point match = findButton(screenshot, action.imagePath, false);
            if (match.x == -1) continue;

            POINT pt = { match.x, match.y };
            ClientToScreen(hwnd, &pt);
            pt = { pt.x + action.x2, pt.y + action.y2 };
            executeActionAtPoint(pt, action.actionName);
            action.executed = true;
            performDelay(action.delay);
        }
        else if (action.type == BehaviorType::FreeClick)
        {
            POINT pt = { action.x, action.y };

            executeActionAtPoint(pt, action.actionName);
            performDelay(action.delay);
        }
        else if (action.type == BehaviorType::Drag)
        {
            POINT from = { action.x, action.y };
            action.executed = true;
            executeActionAtPoint(from, action.actionName + ":" + std::to_string(action.x2) + "," + std::to_string(action.y2));
            performDelay(action.delay);
        }

        // ��ťδ����ɹ����������һ״̬
        if (action.type == BehaviorType::ImageClick && action.executed == false)
            continue;

        if (!action.nextState.empty())
        {
            currentState = action.nextState;

            // ������ж�����ִ�б��
            for (auto& a : statePtr->actions)
                a.executed = false;

            std::cout << "�л�״̬����" << currentState << std::endl;
            break;
        }
    }
}