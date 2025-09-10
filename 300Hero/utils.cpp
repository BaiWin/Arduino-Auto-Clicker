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

    // ����
    DeleteObject(hbmScreen);
    DeleteDC(hdcMemDC);
    ReleaseDC(hwnd, hdcWindow);

    return mat;
}

// ʹ�� OpenCV ���а�ťƥ��
cv::Point findButton(const cv::Mat& screenshot, const std::string& imagePath, bool isResize, double threshold, bool useColorMatch)
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
    /*std::cout << "Screenshot size: " << src.cols << "x" << src.rows
        << ", Template size: " << temp.cols << "x" << temp.rows << std::endl;*/

    // buttonTemplate ���� screenshot��ģ��̫��
    if (temp.cols > src.cols || temp.rows > src.rows)
    {
        std::cerr << "ģ��ͼ��ߴ���ڽ�ͼ��" << std::endl;
        return { -1, -1 };
    }

    // ���ģ��ͼ�Ǵ� Windows Snipping Tool ���ģ��� DPI ���ţ��������Ż�ԭʼ����
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
        cv::Point matchInOriginal;
        if (isResize)
        {
            matchInOriginal = cv::Point(
                static_cast<int>(matchInAdjusted.x * dpiScale),
                static_cast<int>(matchInAdjusted.y * dpiScale)
            );    // 100% ��ͼƬ���� * DPI
        }
        else // ����������
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
            return FALSE; // �ҵ���ֹͣö��
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
        std::cerr << "�޷��򿪴���: " << comPort << std::endl;
        return;
    }

    DWORD bytesWritten;
    WriteFile(hSerial, command.c_str(), command.length(), &bytesWritten, NULL);
    WriteFile(hSerial, "\n", 1, &bytesWritten, NULL);  // �Ӹ����з����� readStringUntil('\n')

    CloseHandle(hSerial);
}

POINT getCurrentMousePos()
{
    POINT curPos;
    GetCursorPos(&curPos); // ��ȡ��ǰ�����Ļ����
    return curPos;
}

void moveToScreenPoint(POINT targetPt)
{
    POINT curPos = getCurrentMousePos();
    int dx = (targetPt.x - curPos.x);
    int dy = (targetPt.y - curPos.y);

    std::string moveCmd = "move:" + std::to_string(dx) + "," + std::to_string(dy);

    std::cout << "��ǰ��꣺" << curPos.x << ", " << curPos.y << std::endl;
    std::cout << "Ŀ�����꣺" << targetPt.x << ", " << targetPt.y << std::endl;
    std::cout << "����ƶ���" << dx << ", " << dy << std::endl;

    sendSerialCommand(moveCmd);
}

// �����ť
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
    std::cout << "��ǰ��꣺" << curPos.x << ", " << curPos.y << std::endl;
}

void keepWindowInForeground(HWND hwnd)
{
    while (true)
    {
        if (IsIconic(hwnd))
        {
            ShowWindow(hwnd, SW_RESTORE); // ��ԭ����
        }

        SetForegroundWindow(hwnd); // ����Ϊǰ̨����
        std::this_thread::sleep_for(std::chrono::seconds(10)); // ÿ��ִ��һ��
    }
}


// ��� screen_capture_lite ͷ�ļ�
#include "ScreenCapture.h"
#include "internal/SCCommon.h"

#include <algorithm>
#undef min
#undef max

using namespace SL::Screen_Capture;

// ȫ��ͼ��ͬ����־
std::atomic<bool> g_done(false);
cv::Mat g_capturedImage;

// ������ͼ�̣߳�ֻ����һ�Σ�
void startCapture(HWND hwnd)
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

// ץһ֡������ cv::Mat
cv::Mat captureWindowToMat(HWND hwnd)
{
    g_done = false;

    auto cfg = CreateCaptureConfiguration([]() {
        return GetMonitors();  // �ĳ�ȫ����ͼ
        });

    cfg->onNewFrame([&](const Image& img, const Monitor&) {
        cv::Mat temp(Height(img), Width(img), CV_8UC4, (void*)StartSrc(img), RowStride(img));
        g_capturedImage = temp.clone();
        g_done = true;

        std::cout << "����ɹ����ߴ�: " << Width(img) << "x" << Height(img) << std::endl;
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
        std::cout << "�ɹ�����ͼ��" << std::endl;
    else
        std::cout << "ͼ��Ϊ��" << std::endl;

    return g_capturedImage.clone();
}

cv::Mat captureWindowFromScreen(HWND hwnd)
{
    g_done = false;

    auto cfg = CreateCaptureConfiguration([]() {
        return GetMonitors();  // ȫ��
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
    w = std::min(w, g_capturedImage.cols - x);
    h = std::min(h, g_capturedImage.rows - y);

    cv::Rect roi(x, y, w, h);
    return g_capturedImage(roi).clone();
}