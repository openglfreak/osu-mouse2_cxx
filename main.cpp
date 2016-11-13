#include "common.h"

#include <tchar.h>
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <psapi.h>
#include <shellapi.h>

#include <gdiplus.h>
using namespace Gdiplus;

extern bool accelDisabled;
extern INT mouseTresholds[3];
void saveAccel();
void enableAccel();
void disableAccel();
void resetAccel();

bool registerHooks();
void unregisterHooks();

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HWND hWnd;
HINSTANCE hInstance;
SIZE windowSize;
const TCHAR wndClassName[] = _T("osu_mouse2");
WNDCLASSEX wndClassEx = { sizeof(WNDCLASSEX), 0, WndProc, 0, 0, NULL, NULL, NULL, (HBRUSH)COLOR_WINDOW, NULL, &wndClassName[0], NULL };
#define APPWM_ICONNOTIFY (WM_APP + 1)
NOTIFYICONDATA ntfIcoData = { sizeof(NOTIFYICONDATA), NULL, 0, NIF_MESSAGE | NIF_TIP, APPWM_ICONNOTIFY, NULL, _T("osu!mouse2 ready"), 0, 0, _T(""), NOTIFYICON_VERSION, _T(""), NIIF_NONE, 0 };
HMENU hPopupMenu;

HWND hButtonEnableAccel;
HWND hButtonDisableAccel;
HWND hButtonExit;
HWND hLabelAbout;
HWND hLabelInfo;
HFONT hFont;
HFONT hFontSmall;
HFONT hFontBold;

Bitmap* logo;

void ForegroundWindowChange(HWND hWnd)
{
    if (accelDisabled)
    {
        resetAccel();
        if (mouseTresholds[2] == 0)
            EnableWindow(hButtonDisableAccel, FALSE);
        else
            EnableWindow(hButtonEnableAccel, FALSE);
        SetWindowText(hLabelInfo, _T("osu! unfocused"));
    }
    else
    {
        static TCHAR buffer[MAX_PATH];
        static DWORD nChars;

        #if not NO_CHECK_TITLE
        nChars = GetWindowText(hWnd, &buffer[0], 6);
        DBG_OUT(&buffer[0]);
        if (nChars != 4 || _tcscmp(&buffer[0], _T("osu!")) != 0)
            return;
        DBG_OUTT("title matched");
        #endif

        #if not NO_CHECK_PROCESS
        static DWORD procID;
        static HANDLE hProcess;

        GetWindowThreadProcessId(hWnd, &procID);
        hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, procID);
        if (!hProcess)
            return;
        nChars = GetProcessImageFileName(hProcess, &buffer[0], MAX_PATH);
        CloseHandle(hProcess);

        if (nChars == 0 || _tcscmp(&buffer[_tcslen(&buffer[0]) - 8], _T("osu!.exe")) != 0)
            return;
        DBG_OUTT("exe matched");
        #endif

        disableAccel();
        EnableWindow(hButtonDisableAccel, FALSE);
        EnableWindow(hButtonEnableAccel, TRUE);
        SetWindowText(hLabelInfo, _T("osu! focused, acceleration disabled"));
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static Graphics* g;
    switch(uMsg)
    {
        case WM_CREATE:
            g = Graphics::FromHWND(hWnd);
            g->SetInterpolationMode(InterpolationModeNearestNeighbor);
            hButtonEnableAccel = CreateWindow(_T("BUTTON"), _T("Enable Accel"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 12, 189, 100, 27, hWnd, NULL, hInstance, NULL);
            hButtonDisableAccel = CreateWindow(_T("BUTTON"), _T("Disable Accel"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 118, 189, 100, 27, hWnd, NULL, hInstance, NULL);
            if (mouseTresholds[2] == 0)
                EnableWindow(hButtonDisableAccel, FALSE);
            else
                EnableWindow(hButtonEnableAccel, FALSE);
            hButtonExit = CreateWindow(_T("BUTTON"), _T("Exit"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 65, 222, 100, 27, hWnd, NULL, hInstance, NULL);
            hLabelAbout = CreateWindow(_T("STATIC"), _T("osu!mouse2 v1.0 - (c) openglfreak 2016"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_CENTER, 0, 149, 229, 17, hWnd, NULL, hInstance, NULL);
            hLabelInfo = CreateWindow(_T("STATIC"), _T("Ready, focus osu!"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_CENTER, 0, 162, 229, 24, hWnd, NULL, hInstance, NULL);
            hFont = CreateFont(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Microsoft Sans Serif"));
            SendMessage(hButtonEnableAccel, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hButtonDisableAccel, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hButtonExit, WM_SETFONT, (WPARAM)hFont, TRUE);
            hFontSmall = CreateFont(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Microsoft Sans Serif"));
            SendMessage(hLabelAbout, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            hFontBold = CreateFont(21, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Microsoft Sans Serif"));
            SendMessage(hLabelInfo, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            break;
        case WM_CLOSE:
            #if not DEBUG
            ShowWindow(hWnd, SW_HIDE);
            #endif // not DEBUG
            return 0;
        case WM_DESTROY:
            DestroyWindow(hButtonEnableAccel);
            DestroyWindow(hButtonDisableAccel);
            DestroyWindow(hButtonExit);
            DestroyWindow(hLabelAbout);
            DestroyWindow(hLabelInfo);
            DeleteObject(hFont);
            DeleteObject(hFontSmall);
            DeleteObject(hFontBold);
            delete g;
            break;
        #if not DEBUG
        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
                ShowWindow(hWnd, SW_HIDE);
            break;
        #endif // not DEBUG
        case WM_SIZE:
            {
                RECT temp;
                GetClientRect(hWnd, &temp);
                windowSize.cx = temp.right - temp.left;
                windowSize.cy = temp.bottom - temp.top;
                InvalidateRect(hWnd, NULL, FALSE);
            }
            break;
        case WM_PAINT:
            if (g && windowSize.cx != 0 && windowSize.cy != 0)
            {
                DefWindowProc(hWnd, uMsg, wParam, lParam);
                g->DrawImage(logo, (INT)windowSize.cx / 5, windowSize.cy * 4 / 21, windowSize.cx * 3 / 5, windowSize.cy * 4 / 21); // Trial and error
            }
            return 0;
        case WM_COMMAND:
            if (lParam)
            {
                if (HIWORD(wParam) == BN_CLICKED)
                {
                    if (lParam == (LPARAM)hButtonEnableAccel)
                    {
                        enableAccel();
                        EnableWindow(hButtonEnableAccel, FALSE);
                        EnableWindow(hButtonDisableAccel, TRUE);
                    }
                    else if (lParam == (LPARAM)hButtonDisableAccel)
                    {
                        disableAccel();
                        EnableWindow(hButtonDisableAccel, FALSE);
                        EnableWindow(hButtonEnableAccel, TRUE);
                    }
                    else if (lParam == (LPARAM)hButtonExit)
                        PostQuitMessage(0);
                }
            }
            else if (HIWORD(wParam) == 0)
                switch (LOWORD(wParam))
                {
                    case IDM_TRAY_POPUPMENU_ENABLE:
                        enableAccel();
                        EnableWindow(hButtonEnableAccel, FALSE);
                        EnableWindow(hButtonDisableAccel, TRUE);
                        break;
                    case IDM_TRAY_POPUPMENU_DISABLE:
                        disableAccel();
                        EnableWindow(hButtonDisableAccel, FALSE);
                        EnableWindow(hButtonEnableAccel, TRUE);
                        break;
                    case IDM_TRAY_POPUPMENU_EXIT:
                        PostQuitMessage(0);
                        break;
                }
            return 0;
        case APPWM_ICONNOTIFY:
            switch(lParam)
            {
                #if not DEBUG
                case WM_LBUTTONUP:
                    ShowWindow(hWnd, SW_SHOW);
                    SetForegroundWindow(hWnd);
                    SetActiveWindow(hWnd);
                    break;
                #endif // not DEBUG
                case WM_RBUTTONUP:
                    {
                        POINT p;
                        GetCursorPos(&p);
                        SetForegroundWindow(hWnd); // **Workaround** PopupMenu doesn't close without this
                        SetActiveWindow(hWnd); // I don't think I need this
                        TrackPopupMenu(hPopupMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hWnd, NULL);
                    }
                    break;
            }
            return 0;
        #if DEBUG && 0
        default:
            printf("%u %04X.%04X %08X\n", (unsigned int)uMsg, (unsigned int)HIWORD(wParam),(unsigned int)LOWORD(wParam), (unsigned int)lParam);
        #endif // DEBUG && 0
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

Bitmap* loadImageResource(HINSTANCE hinstance, LPCTSTR name, LPCTSTR type)
{
    HRSRC hRsrc = FindResource(hInstance, name, type);
    if (hRsrc == NULL)
        return NULL;
    DWORD rsrcSize = SizeofResource(hInstance, hRsrc);
    LPVOID rsrcPointer = LockResource(LoadResource(hInstance, hRsrc));
    HGLOBAL rsrcGlobal = GlobalAlloc(GMEM_MOVEABLE, rsrcSize);
    if (rsrcGlobal == NULL)
        return NULL;
    Bitmap* ret = NULL;
    void* data = GlobalLock(rsrcGlobal);
    memcpy(data, rsrcPointer, rsrcSize);
    IStream* stream;
    if (CreateStreamOnHGlobal(rsrcGlobal, FALSE, &stream) == S_OK)
    {
        ret = Bitmap::FromStream(stream);
        stream->Release();
    }
    GlobalUnlock(rsrcGlobal);
    GlobalFree(rsrcGlobal);
    return ret;
}

ULONG_PTR gdiplusToken;

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    ::hInstance = hInstance;

    saveAccel();
    registerHooks();

    GdiplusStartupInput gdipTempInp;
    GdiplusStartup(&gdiplusToken, &gdipTempInp, NULL);

    logo = loadImageResource(hInstance, MAKEINTRESOURCE(IDB_LOGO), _T("PNG"));

    InitCommonControls();

    wndClassEx.hInstance = hInstance;
    wndClassEx.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LOGO));
    wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClassEx.hIconSm = wndClassEx.hIcon;

    if (!RegisterClassEx(&wndClassEx))
        return 0;

    RECT size;
    size.left = 0;
    size.top = 0;
    windowSize.cx = size.right = 229;
    windowSize.cy = size.bottom = 261;
    AdjustWindowRectEx(&size, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, FALSE, WS_EX_TOOLWINDOW | WS_EX_TOPMOST);
    size.right -= size.left;
    size.bottom -= size.top;
    hWnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, wndClassName, _T("osu!mouse2"), WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, size.right, size.bottom, HWND_DESKTOP, NULL, hInstance, NULL);
    ShowWindow(hWnd, nCmdShow);
    SetForegroundWindow(hWnd);
    SetActiveWindow(hWnd);

    hPopupMenu = LoadMenu(NULL, MAKEINTRESOURCE(IDR_TRAY_POPUPMENU));
    hPopupMenu = GetSubMenu(hPopupMenu, 0);

    ntfIcoData.hWnd = hWnd;
    ntfIcoData.uFlags |= NIF_ICON;
    ntfIcoData.hIcon = wndClassEx.hIcon;
    Shell_NotifyIcon(NIM_ADD, &ntfIcoData);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Shell_NotifyIcon(NIM_DELETE, &ntfIcoData);

    delete logo;

    GdiplusShutdown(gdiplusToken);

    unregisterHooks();
    resetAccel();
    return msg.wParam;
}
