#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <Windows.h>

extern double dpiScale;

cv::Mat captureWindow(HWND hwnd);
cv::Point findButton(const cv::Mat& screenshot, const std::string& imagePath, bool isResize, double threshold = 0.6, bool useColorMatch = false);
DWORD FindPIDByExeName(const std::wstring& exeName);
HWND FindMainWindowByPID(DWORD pid);
void sendSerialCommand(const std::string& command);
POINT getCurrentMousePos();
void moveToScreenPoint(POINT targetPt);
void executeActionAtPoint(POINT targetPt, const std::string& clickType);
void performDelay(int milliseconds);
void PrintMousePos();
void keepWindowInForeground(HWND hwnd);
void startCapture(HWND hwnd);
cv::Mat captureWindowToMat(HWND hwnd);
cv::Mat captureWindowFromScreen(HWND hwnd);