#include "common.h"

#include <windows.h>

void ForegroundWindowChange(HWND hWnd);

static bool hooked = false;
static HWND hForegroundWnd;
static HWINEVENTHOOK hWinEventHooks[3];

void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hWnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
    if (hWnd != hForegroundWnd)
    {
        hForegroundWnd = hWnd;
        ForegroundWindowChange(hWnd);
    }
}

bool registerHooks()
{
    hForegroundWnd = GetForegroundWindow();
    if (!(hWinEventHooks[0] = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT)))
        return false;
    hWinEventHooks[1] = SetWinEventHook(EVENT_SYSTEM_CAPTURESTART, EVENT_SYSTEM_CAPTURESTART, NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
    hooked = true;
    return true;
}

void unregisterHooks()
{
    hooked = false;
    UnhookWinEvent(hWinEventHooks[0]);
    if (hWinEventHooks[1])
        UnhookWinEvent(hWinEventHooks[1]);
}
