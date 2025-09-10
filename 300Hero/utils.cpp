#include "utils.h"
#include <TlHelp32.h>
#include <iostream>

double dpiScale = 1.5; // 150%
const std::string comPort = "COM7";

cv::Mat captureWindow(HWND hwnd)
{
    RECT rc;
    GetClientRect(hwnd, &rc);

    HDC hdcWindow = GetDC(hwnd);
    HDC hdcMemDC = CreateCompatibleDC(hdcWindow);
    HBITMAP hbmScreen = CreateCompatibleBitmap(hdcWindow, rc.right, rc.bottom);
    SelectObject(hdcMemDC, hbmScreen);

    BitBlt(hdcMemDC, 0, 0, rc.right, rc.bottom, hdcWindow, 0, 0, SRCCOPY);

    BITMAP bmp;
    GetObject(hbmScreen, sizeof(BITMAP), &bmp);

    BITMAPINFOHEADER bi;
    ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmp.bmWidth;
    bi.biHeight = -bmp.bmHeight;  // top-down
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;

    cv::Mat mat(bmp.bmHeight, bmp.bmWidth, CV_8UC4);
    GetDIBits(hdcWindow, hbmScreen, 0, (UINT)bmp.bmHeight, mat.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    // 清理
    DeleteObject(hbmScreen);
    DeleteDC(hdcMemDC);
    ReleaseDC(hwnd, hdcWindow);

    return mat;
}

// 使用 OpenCV 进行按钮匹配
cv::Point findButton(const cv::Mat& screenshot, const std::string& imagePath, bool isResize, double threshold, bool useColorMatch)
{
    const cv::Mat tmpl = cv::imread(imagePath, cv::IMREAD_UNCHANGED);
    if (tmpl.empty())
    {
        std::cerr << "找不到模板图像: " << imagePath << std::endl;
        return { -1, -1 };
    }

    cv::Mat src = screenshot.clone();
    cv::Mat temp = tmpl.clone();

    if (src.type() != temp.type())
    {
        std::cerr << "图像类型不一致，正在转换..." << std::endl;

        // 如果 useGrayMatch 启用，统一转为灰度
        if (useColorMatch)
        {
            // 如果不是灰度匹配，也转换为 BGR 类型一致
            if (src.channels() == 4) cv::cvtColor(src, src, cv::COLOR_BGRA2BGR);
            if (temp.channels() == 4) cv::cvtColor(temp, temp, cv::COLOR_BGRA2BGR);
        }
        else
        {
            if (src.channels() == 4) cv::cvtColor(src, src, cv::COLOR_BGRA2GRAY);
            else if (src.channels() == 3) cv::cvtColor(src, src, cv::COLOR_BGR2GRAY);

            if (temp.channels() == 4) cv::cvtColor(temp, temp, cv::COLOR_BGRA2GRAY);
            else if (temp.channels() == 3) cv::cvtColor(temp, temp, cv::COLOR_BGR2GRAY);
        }
    }

    // 打印图像尺寸，方便调试 DPI 错误
    /*std::cout << "Screenshot size: " << src.cols << "x" << src.rows
        << ", Template size: " << temp.cols << "x" << temp.rows << std::endl;*/

    // buttonTemplate 大于 screenshot（模板太大）
    if (temp.cols > src.cols || temp.rows > src.rows)
    {
        std::cerr << "模板图像尺寸大于截图！" << std::endl;
        return { -1, -1 };
    }

    // 如果模板图是从 Windows Snipping Tool 来的（受 DPI 缩放），先缩放回原始比例
    static cv::Mat adjustedTemplate;
    //, adjustedScreenshot;
    //cv::resize(temp, adjustedTemplate, cv::Size(), 1.0 / dpiScale, 1.0 / dpiScale); // 150% -> 100% adjustedTemplate
    //cv::resize(src, adjustedScreenshot, cv::Size(), dpiScale, dpiScale);

    if (isResize)
        cv::resize(temp, adjustedTemplate, cv::Size(), 1.0 / dpiScale, 1.0 / dpiScale); // 150% -> 100% adjustedTemplate
    else
        adjustedTemplate = temp;

    cv::Mat result;
    try
    {
        cv::matchTemplate(src, adjustedTemplate, result, cv::TM_CCOEFF_NORMED);
    }
    catch (const cv::Exception& e)
    {
        std::cerr << "OpenCV 异常: " << e.what() << std::endl;
        return { -1, -1 };
    }

    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

    std::cout << "Match score (" << imagePath << "): " << std::fixed << std::setprecision(1) << maxVal << std::endl;

    if (maxVal >= threshold) // 添加模板中心偏移
    {
        cv::Point centerOffset(adjustedTemplate.cols / 2, adjustedTemplate.rows / 2); // 100%缩放的按钮中心
        std::cout << "图片坐标：" << (maxLoc + centerOffset).x << ", " << (maxLoc + centerOffset).y << std::endl;
        cv::Point matchInAdjusted = maxLoc + centerOffset;
        cv::Point matchInOriginal;
        if (isResize)
        {
            matchInOriginal = cv::Point(
                static_cast<int>(matchInAdjusted.x * dpiScale),
                static_cast<int>(matchInAdjusted.y * dpiScale)
            );    // 100% 的图片坐标 * DPI
        }
        else // 这里有问题
        {
            matchInOriginal = matchInAdjusted;
        }

        return matchInOriginal;
    }

    return { -1, -1 };
}

DWORD FindPIDByExeName(const std::wstring& exeName)
{
    DWORD pid = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32W pe;
        pe.dwSize = sizeof(pe);
        if (Process32FirstW(hSnapshot, &pe))
        {
            do
            {
                if (_wcsicmp(pe.szExeFile, exeName.c_str()) == 0)
                {
                    pid = pe.th32ProcessID;
                    break;
                }
            } while (Process32NextW(hSnapshot, &pe));
        }
        CloseHandle(hSnapshot);
    }
    return pid;
}

HWND FindMainWindowByPID(DWORD pid)
{
    HWND hwnd = nullptr;

    struct HandleData
    {
        DWORD pid;
        HWND hwnd;
    } data = { pid, nullptr };

    auto enumCallback = [](HWND hWnd, LPARAM lParam) -> BOOL {
        DWORD windowPID;
        GetWindowThreadProcessId(hWnd, &windowPID);
        auto pData = reinterpret_cast<HandleData*>(lParam);

        if (windowPID == pData->pid && GetWindow(hWnd, GW_OWNER) == NULL && IsWindowVisible(hWnd))
        {
            pData->hwnd = hWnd;
            return FALSE; // 找到后停止枚举
        }
        return TRUE;
    };

    EnumWindows(enumCallback, reinterpret_cast<LPARAM>(&data));
    return data.hwnd;
}

void sendSerialCommand(const std::string& command)
{
    std::string fullPort = "\\\\.\\" + comPort;

    HANDLE hSerial = CreateFileA(fullPort.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (hSerial == INVALID_HANDLE_VALUE)
    {
        std::cerr << "无法打开串口: " << comPort << std::endl;
        return;
    }

    DWORD bytesWritten;
    WriteFile(hSerial, command.c_str(), command.length(), &bytesWritten, NULL);
    WriteFile(hSerial, "\n", 1, &bytesWritten, NULL);  // 加个换行符触发 readStringUntil('\n')

    CloseHandle(hSerial);
}

POINT getCurrentMousePos()
{
    POINT curPos;
    GetCursorPos(&curPos); // 获取当前鼠标屏幕坐标
    return curPos;
}

void moveToScreenPoint(POINT targetPt)
{
    POINT curPos = getCurrentMousePos();
    int dx = (targetPt.x - curPos.x);
    int dy = (targetPt.y - curPos.y);

    std::string moveCmd = "move:" + std::to_string(dx) + "," + std::to_string(dy);

    std::cout << "当前鼠标：" << curPos.x << ", " << curPos.y << std::endl;
    std::cout << "目标坐标：" << targetPt.x << ", " << targetPt.y << std::endl;
    std::cout << "相对移动：" << dx << ", " << dy << std::endl;

    sendSerialCommand(moveCmd);
}

// 点击按钮
void executeActionAtPoint(POINT targetPt, const std::string& clickType)
{
    moveToScreenPoint(targetPt);
    Sleep(400);
    if (!clickType.empty())
    {
        sendSerialCommand(clickType);
    }
}

void performDelay(int milliseconds)
{
    if (milliseconds > 0)
    {
        Sleep(milliseconds);
    }
}

void PrintMousePos()
{
    POINT curPos;
    GetCursorPos(&curPos);
    std::cout << "当前鼠标：" << curPos.x << ", " << curPos.y << std::endl;
}

void keepWindowInForeground(HWND hwnd)
{
    while (true)
    {
        if (IsIconic(hwnd))
        {
            ShowWindow(hwnd, SW_RESTORE); // 还原窗口
        }

        SetForegroundWindow(hwnd); // 设置为前台窗口
        std::this_thread::sleep_for(std::chrono::seconds(10)); // 每秒执行一次
    }
}


// 添加 screen_capture_lite 头文件
#include "ScreenCapture.h"
#include "internal/SCCommon.h"

#include <algorithm>
#undef min
#undef max

using namespace SL::Screen_Capture;

// 全局图像及同步标志
std::atomic<bool> g_done(false);
cv::Mat g_capturedImage;

// 启动截图线程（只调用一次）
void startCapture(HWND hwnd)
{
    auto cfg = CreateCaptureConfiguration([=]() {
        auto all = GetWindows();
        for (auto& w : all)
        {
            if (w.Handle == (size_t)hwnd)  // 找到对应 HWND
                return std::vector<Window>{w};
        }
        return std::vector<Window>{};
        });

    g_done = false;
    cfg->onNewFrame([&](const Image& img, const Window&) {
        cv::Mat temp(Height(img), Width(img), CV_8UC4,
        (void*)StartSrc(img), RowStride(img));
    g_capturedImage = temp.clone();
    g_done = true;
        });

    auto grab = cfg->start_capturing();
    grab->setFrameChangeInterval(std::chrono::milliseconds(100));
}

// 抓一帧，返回 cv::Mat
cv::Mat captureWindowToMat(HWND hwnd)
{
    g_done = false;

    auto cfg = CreateCaptureConfiguration([]() {
        return GetMonitors();  // 改成全屏截图
        });

    cfg->onNewFrame([&](const Image& img, const Monitor&) {
        cv::Mat temp(Height(img), Width(img), CV_8UC4, (void*)StartSrc(img), RowStride(img));
        g_capturedImage = temp.clone();
        g_done = true;

        std::cout << "捕获成功，尺寸: " << Width(img) << "x" << Height(img) << std::endl;
        });

    auto grabber = cfg->start_capturing();
    grabber->setFrameChangeInterval(std::chrono::milliseconds(100));

    int waited = 0;
    while (!g_done && waited < 2000)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        waited += 10;
    }

    if (!g_capturedImage.empty())
        std::cout << "成功返回图像" << std::endl;
    else
        std::cout << "图像为空" << std::endl;

    return g_capturedImage.clone();
}

cv::Mat captureWindowFromScreen(HWND hwnd)
{
    g_done = false;

    auto cfg = CreateCaptureConfiguration([]() {
        return GetMonitors();  // 全屏
        });

    cfg->onNewFrame([&](const Image& img, const Monitor&) {
        cv::Mat full(Height(img), Width(img), CV_8UC4, (void*)StartSrc(img), RowStride(img));
        g_capturedImage = full.clone();
        g_done = true;
        });

    auto grabber = cfg->start_capturing();
    grabber->setFrameChangeInterval(std::chrono::milliseconds(100));

    int waited = 0;
    while (!g_done && waited < 2000)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        waited += 10;
    }

    if (g_capturedImage.empty())
    {
        std::cout << "截图失败" << std::endl;
        return {};
    }

    // 裁剪窗口区域
    RECT rect;
    if (!GetWindowRect(hwnd, &rect))
    {
        std::cout << "无法获取窗口位置" << std::endl;
        return {};
    }

    int x = rect.left;
    int y = rect.top;
    int w = rect.right - rect.left;
    int h = rect.bottom - rect.top;

    // 边界检查
    x = std::max(0, x);
    y = std::max(0, y);
    w = std::min(w, g_capturedImage.cols - x);
    h = std::min(h, g_capturedImage.rows - y);

    cv::Rect roi(x, y, w, h);
    return g_capturedImage(roi).clone();
}