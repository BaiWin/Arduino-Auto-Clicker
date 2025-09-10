#include "utils.h"
#include "300config.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include <conio.h>  // Windows �����ڼ���������

bool g_foregroundThreadStarted = false;

void Run300Game()
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

    if (hwnd && !g_foregroundThreadStarted)
    {
        g_foregroundThreadStarted = true;
        std::thread t(keepWindowInForeground, hwnd);
        t.detach(); // ��̨����
    }

    if (!hwnd || hwnd != GetForegroundWindow()) return;

    // Check2 �����ޱ߿�
    /*HWND fgWnd = GetForegroundWindow();
    DWORD fgPID = 0;
    GetWindowThreadProcessId(fgWnd, &fgPID);
    if (fgPID != pid) { return; }*/

    cv::Mat screenshot = captureWindow(hwnd);

    //cv::imwrite("screenshot.png", screenshot);

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

    for (auto& action : statePtr->actions)
    {
        if (action.type == BehaviorType::ImageClick)
        {
            bool isResize = true;
            if (action.imageCaptureMethod == 1)
            {
                screenshot = captureWindowFromScreen(hwnd);
                isResize = false;
                //cv::imwrite("screenshot.png", screenshot);
            }

            cv::Mat tmpl = cv::imread(action.imagePath, cv::IMREAD_UNCHANGED);
            if (tmpl.empty()) continue;

            cv::Point match = findButton(screenshot, action.imagePath, isResize, action.threshold);
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
            ClientToScreen(hwnd, &pt);
            executeActionAtPoint(pt, action.actionName);
            performDelay(action.delay);
        }
        else if (action.type == BehaviorType::RandomClick)
        {
            int rx = 900 + rand() % (1000 - 900); // [900, 999]
            int ry = 500 + rand() % (600 - 500); // [500, 599]
            POINT pt = { rx, ry };
            executeActionAtPoint(pt, action.actionName);
            performDelay(action.delay);
        }
        else if (action.type == BehaviorType::Hover)
        {
            POINT pt;

            if (!action.imagePath.empty())
            {
                cv::Mat tmpl = cv::imread(action.imagePath, cv::IMREAD_UNCHANGED);
                if (tmpl.empty()) continue;

                cv::Point match = findButton(screenshot, action.imagePath, true);
                if (match.x == -1) continue;

                pt = { match.x, match.y };
                ClientToScreen(hwnd, &pt);
            }
            else
            {
                pt = { action.x, action.y };
                ClientToScreen(hwnd, &pt);
            }
            // �ƶ�����ȥ�������
            executeActionAtPoint(pt, action.actionName);
            action.executed = true;

            // ��ͣ�ȴ���ʹ�� delay��
            performDelay(action.delay);
        }
        else if (action.type == BehaviorType::Drag)
        {
            // ����չ���� (x,y) �ϵ� (x2,y2)
            POINT from = { action.x, action.y };
            POINT to = { action.x2, action.y2 };
            ClientToScreen(hwnd, &from);
            ClientToScreen(hwnd, &to);
            action.executed = true;
            // ����Ҫ�� utils ����� dragMouse(from, to) ����
            // dragMouse(from, to);
            performDelay(action.delay);
            
            break;
        }
        else if (action.type == BehaviorType::KeyPressImage)
        {
            POINT pt;

            if (!action.imagePath.empty())
            {
                cv::Mat tmpl = cv::imread(action.imagePath, cv::IMREAD_UNCHANGED);
                if (tmpl.empty()) continue;

                cv::Point match = findButton(screenshot, action.imagePath, true);
                if (match.x == -1) continue;

                pt = { match.x, match.y };
                ClientToScreen(hwnd, &pt);
            }
            else
            {
                pt = { action.x, action.y };
                ClientToScreen(hwnd, &pt);
            }

            // �ƶ�����ȥ�������
            // ���ͼ���ָ�ת���ַ���
            pt = { pt.x + action.x2, pt.y + action.y2 };
            char keyChar = static_cast<char>(action.virtualKey);
            executeActionAtPoint(pt, action.actionName + ":" + keyChar);
            action.executed = true;
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