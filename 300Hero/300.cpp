#include "utils.h"
#include "300config.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include <conio.h>  // Windows 下用于检测键盘输入

bool g_foregroundThreadStarted = false;

void Run300Game()
{
    // 初始化只做一次
    static bool initialized = false;
    if (!initialized)
    {
        for (auto& mode : myGame.modes)
        {
            if (mode.modeName == selectedModeName)
            {
                std::cout << "当前模式：" << selectedModeName << std::endl;
                currentMode = &mode;
                currentState = mode.initialState;
                break;
            }
        }

        if (!currentMode)
        {
            std::cerr << "找不到模式：" << selectedModeName << std::endl;
            return;
        }

        initialized = true;
    }

    SetProcessDPIAware();

    DWORD pid = FindPIDByExeName(std::wstring(myGame.exeName.begin(), myGame.exeName.end()));
    if (pid == 0) return;

    // Check1 用于有边框
    HWND hwnd = FindMainWindowByPID(pid);

    if (hwnd && !g_foregroundThreadStarted)
    {
        g_foregroundThreadStarted = true;
        std::thread t(keepWindowInForeground, hwnd);
        t.detach(); // 后台运行
    }

    if (!hwnd || hwnd != GetForegroundWindow()) return;

    // Check2 用于无边框
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
        std::cout << "未找到状态：" << currentState << std::endl;
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
            // 移动鼠标过去但不点击
            executeActionAtPoint(pt, action.actionName);
            action.executed = true;

            // 悬停等待（使用 delay）
            performDelay(action.delay);
        }
        else if (action.type == BehaviorType::Drag)
        {
            // 可扩展：从 (x,y) 拖到 (x2,y2)
            POINT from = { action.x, action.y };
            POINT to = { action.x2, action.y2 };
            ClientToScreen(hwnd, &from);
            ClientToScreen(hwnd, &to);
            action.executed = true;
            // 你需要在 utils 中添加 dragMouse(from, to) 函数
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

            // 移动鼠标过去但不点击
            // 发送键盘指令（转成字符）
            pt = { pt.x + action.x2, pt.y + action.y2 };
            char keyChar = static_cast<char>(action.virtualKey);
            executeActionAtPoint(pt, action.actionName + ":" + keyChar);
            action.executed = true;
            performDelay(action.delay);
        }

        // 按钮未点击成功不会进入下一状态
        if (action.type == BehaviorType::ImageClick && action.executed == false)
            continue;

        if (!action.nextState.empty())
        {
            currentState = action.nextState;

            // 清除所有动作已执行标记
            for (auto& a : statePtr->actions)
                a.executed = false;

            std::cout << "切换状态到：" << currentState << std::endl;
            break;
        }
    }
}