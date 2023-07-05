#include "pch.h"
#include <iostream>
#include <Windows.h>

HWND hIndicatorWnd = nullptr;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);

    if (processId == GetCurrentProcessId())
    {
        hIndicatorWnd = hwnd;
        return FALSE;
    }

    return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        wchar_t processName[MAX_PATH];
        GetModuleFileNameW(NULL, processName, MAX_PATH);

        std::wcout << L"DLL injected into process: " << processName << std::endl;
        std::wcout << L"Hello, World!" << std::endl;

        // Find the main window of the process
        EnumWindows(EnumWindowsProc, 0);

        if (hIndicatorWnd != nullptr)
        {
            // Create and show the indicator window
            MessageBoxW(hIndicatorWnd, L"DLL Injected!", L"Indicator", MB_OK | MB_ICONINFORMATION);
        }

        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}
