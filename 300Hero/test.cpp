#include <opencv2/opencv.hpp>
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <string>


// ��� screen_capture_lite ͷ�ļ�
#include "ScreenCapture.h"
#include "internal/SCCommon.h"

#include <algorithm>
#undef min
#undef max

using namespace SL::Screen_Capture;

// ȫ��ͼ��ͬ����־
std::atomic<bool> g_done_Test(false);
cv::Mat g_capturedImage_Test;

// ������ͼ�̣߳�ֻ����һ�Σ�
void startCapture_Test(HWND hwnd)
{
    auto cfg = CreateCaptureConfiguration([=]() {
        auto all = GetWindows();
        for (auto& w : all)
        {
            if (w.Handle == (size_t)hwnd)  // �ҵ���Ӧ HWND
                return std::vector<Window>{w};
        }
        return std::vector<Window>{};
        });

    g_done_Test = false;
    cfg->onNewFrame([&](const Image& img, const Window&) {
        cv::Mat temp(Height(img), Width(img), CV_8UC4,
        (void*)StartSrc(img), RowStride(img));
    g_capturedImage_Test = temp.clone();
    g_done_Test = true;
        });

    auto grab = cfg->start_capturing();
    grab->setFrameChangeInterval(std::chrono::milliseconds(100));
}

// ץһ֡������ cv::Mat
cv::Mat captureWindowToMat_Test(HWND hwnd)
{
    g_done_Test = false;

    auto cfg = CreateCaptureConfiguration([]() {
        return GetMonitors();  // �ĳ�ȫ����ͼ
        });

    cfg->onNewFrame([&](const Image& img, const Monitor&) {
        cv::Mat temp(Height(img), Width(img), CV_8UC4, (void*)StartSrc(img), RowStride(img));
        g_capturedImage_Test = temp.clone();
        g_done_Test = true;

        std::cout << "����ɹ����ߴ�: " << Width(img) << "x" << Height(img) << std::endl;
        });

    auto grabber = cfg->start_capturing();
    grabber->setFrameChangeInterval(std::chrono::milliseconds(100));

    int waited = 0;
    while (!g_done_Test && waited < 2000)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        waited += 10;
    }

    if (!g_capturedImage_Test.empty())
        std::cout << "�ɹ�����ͼ��" << std::endl;
    else
        std::cout << "ͼ��Ϊ��" << std::endl;

    return g_capturedImage_Test.clone();
}

cv::Mat captureWindowFromScreen_Test(HWND hwnd)
{
    g_done_Test = false;

    auto cfg = CreateCaptureConfiguration([]() {
        return GetMonitors();  // ȫ��
        });

    cfg->onNewFrame([&](const Image& img, const Monitor&) {
        cv::Mat full(Height(img), Width(img), CV_8UC4, (void*)StartSrc(img), RowStride(img));
        g_capturedImage_Test = full.clone();
        g_done_Test = true;
        });

    auto grabber = cfg->start_capturing();
    grabber->setFrameChangeInterval(std::chrono::milliseconds(100));

    int waited = 0;
    while (!g_done_Test && waited < 2000)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        waited += 10;
    }

    if (g_capturedImage_Test.empty())
    {
        std::cout << "��ͼʧ��" << std::endl;
        return {};
    }

    // �ü���������
    RECT rect;
    if (!GetWindowRect(hwnd, &rect))
    {
        std::cout << "�޷���ȡ����λ��" << std::endl;
        return {};
    }

    int x = rect.left;
    int y = rect.top;
    int w = rect.right - rect.left;
    int h = rect.bottom - rect.top;

    // �߽���
    x = std::max(0, x);
    y = std::max(0, y);
    w = std::min(w, g_capturedImage_Test.cols - x);
    h = std::min(h, g_capturedImage_Test.rows - y);

    cv::Rect roi(x, y, w, h);
    return g_capturedImage_Test(roi).clone();
}

// �����ڽ�ͼΪ cv::Mat
cv::Mat captureWindow_Test(HWND hwnd)
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

    // ����
    DeleteObject(hbmScreen);
    DeleteDC(hdcMemDC);
    ReleaseDC(hwnd, hdcWindow);

    return mat;
}

// ʹ�� OpenCV ���а�ťƥ��
cv::Point findButton_Test(const cv::Mat& screenshot, const std::string& imagePath, double threshold, bool useColorMatch)
{
    const cv::Mat tmpl = cv::imread(imagePath, cv::IMREAD_UNCHANGED);
    if (tmpl.empty())
    {
        std::cerr << "�Ҳ���ģ��ͼ��: " << imagePath << std::endl;
        return { -1, -1 };
    }

    cv::Mat src = screenshot.clone();
    cv::Mat temp = tmpl.clone();

    if (src.type() != temp.type())
    {
        std::cerr << "ͼ�����Ͳ�һ�£�����ת��..." << std::endl;

        // ��� useGrayMatch ���ã�ͳһתΪ�Ҷ�
        if (useColorMatch)
        {
            // ������ǻҶ�ƥ�䣬Ҳת��Ϊ BGR ����һ��
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

    // ��ӡͼ��ߴ磬������� DPI ����
    std::cout << "Screenshot size: " << src.cols << "x" << src.rows
        << ", Template size: " << temp.cols << "x" << temp.rows << std::endl;

    // buttonTemplate ���� screenshot��ģ��̫��
    if (temp.cols > src.cols || temp.rows > src.rows)
    {
        std::cerr << "ģ��ͼ��ߴ���ڽ�ͼ��" << std::endl;
        return { -1, -1 };
    }

    // ���ģ��ͼ�Ǵ� Windows Snipping Tool ���ģ��� DPI ���ţ��������Ż�ԭʼ����
    static cv::Mat adjustedTemplate;
    //, adjustedScreenshot;
    cv::resize(temp, adjustedTemplate, cv::Size(), 1.0 / 1.5, 1.0 / 1.5); // 150% -> 100% adjustedTemplate
    //cv::resize(src, adjustedScreenshot, cv::Size(), dpiScale, dpiScale);

    cv::cvtColor(src, src, cv::COLOR_BGR2GRAY);
    cv::cvtColor(temp, temp, cv::COLOR_BGR2GRAY);

    cv::imshow("Image", temp);
    cv::imshow("Image2", src);

    cv::moveWindow("Image", 2300, 1300);

    cv::Mat result;
    try
    {
        cv::matchTemplate(src, temp, result, cv::TM_CCOEFF_NORMED);
    }
    catch (const cv::Exception& e)
    {
        std::cerr << "OpenCV �쳣: " << e.what() << std::endl;
        return { -1, -1 };
    }

    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

    std::cout << "Match score (" << imagePath << "): " << std::fixed << std::setprecision(1) << maxVal << std::endl;

    if (maxVal >= threshold) // ���ģ������ƫ��
    {
        cv::Point centerOffset(adjustedTemplate.cols / 2, adjustedTemplate.rows / 2); // 100%���ŵİ�ť����
        std::cout << "ͼƬ���꣺" << (maxLoc + centerOffset).x << ", " << (maxLoc + centerOffset).y << std::endl;
        cv::Point matchInAdjusted = maxLoc + centerOffset;
        cv::Point matchInOriginal = cv::Point(
            static_cast<int>(matchInAdjusted.x * 1.5),
            static_cast<int>(matchInAdjusted.y * 1.5)
        );    // 100% ��ͼƬ���� * DPI

        return matchInOriginal;
    }

    return { -1, -1 };
}

DWORD FindPIDByExeName_Test(const std::wstring& exeName)
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

HWND FindMainWindowByPID_Test(DWORD pid)
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
            return FALSE; // �ҵ���ֹͣö��
        }
        return TRUE;
    };

    EnumWindows(enumCallback, reinterpret_cast<LPARAM>(&data));
    return data.hwnd;
}

void sendSerialCommand_Test(const std::string& comPort, const std::string& command)
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
        std::cerr << "�޷��򿪴���: " << comPort << std::endl;
        return;
    }

    DWORD bytesWritten;
    WriteFile(hSerial, command.c_str(), command.length(), &bytesWritten, NULL);
    WriteFile(hSerial, "\n", 1, &bytesWritten, NULL);  // �Ӹ����з����� readStringUntil('\n')

    CloseHandle(hSerial);
}

POINT getCurrentMousePos_Test()
{
    POINT curPos;
    GetCursorPos(&curPos); // ��ȡ��ǰ�����Ļ����
    return curPos;
}

// �����ť
void clickScreenPoint_Test(POINT targetPt, const std::string& clickType)
{
    POINT curPos = getCurrentMousePos_Test();
    int dx = (targetPt.x - curPos.x);
    int dy = (targetPt.y - curPos.y);

    std::string moveCmd = "move:" + std::to_string(dx) + "," + std::to_string(dy);

    std::cout << "��ǰ��꣺" << curPos.x << ", " << curPos.y << std::endl;
    std::cout << "Ŀ�����꣺" << targetPt.x << ", " << targetPt.y << std::endl;
    std::cout << "��������ƶ���" << dx << ", " << dy << std::endl;

    sendSerialCommand_Test("COM7", moveCmd);
    sendSerialCommand_Test("COM7", clickType);
}

int main_Test()
{
    SetProcessDPIAware();

    while (true)
    {
        std::wstring exeName = L"Hearthstone.exe";

        DWORD pid = FindPIDByExeName_Test(exeName);

        if (pid == 0)
        {
            std::cout << "�Ҳ�������!" << std::endl;
            continue;
        }

        HWND hwnd = FindMainWindowByPID_Test(pid);
        if (!hwnd)
        {
            std::cout << "�Ҳ������ھ����" << std::endl;
            continue;
        }

        if (!(GetForegroundWindow() == hwnd))
        {
            continue;
        }

        //cv::Mat screenshot = captureWindow_Test(hwnd);

        UINT dpi = GetDpiForWindow(hwnd);
        std::cout << "���� DPI: " << dpi << std::endl;

        cv::Mat screenshot = captureWindowFromScreen_Test(hwnd);

        std::string path = "resources\\hearthstone\\selectcard.png";

        cv::Mat button = cv::imread(path, cv::IMREAD_UNCHANGED);

        if (screenshot.empty() || button.empty())
        {
            std::cout << "��ͼ��ťͼ����ʧ��" << std::endl;
            continue;
        }

        cv::Point match = findButton_Test(screenshot, path, 0.5, false);

        if (match.x != -1)
        {
            // ת��Ϊ��Ļ����
            POINT pt = { match.x, match.y};
            ClientToScreen(hwnd, &pt);

            std::cout << "�ҵ���ť���ڴ���������: " << match << "����Ļ���꣺" << pt.x << "," << pt.y << std::endl;
            clickScreenPoint_Test(pt, "leftclick");
        }

        /*cv::Mat img = captureWindowFromScreen(hwnd);
        if (img.empty())
        {
            std::cout << "��ͼʧ��\n";
            return 0;
        }*/

        // ���ͼƬ
        //cv::imshow("Image", screenshot);
        //cv::imshow("Image2", button);
        cv::waitKey(0);

        Sleep(3000);
        break;
    }


    return 0;
}