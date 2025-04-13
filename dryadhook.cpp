#include "stdafx.h"
#include "window.h"
#include <stdexcept>
#include <iostream>

HMODULE instance = NULL;

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    std::cout << "[DryadHook] Starting up..." << std::endl;

	Sleep(100000);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        instance = hModule;
        const HANDLE hThread = CreateThread(NULL, 0, ThreadProc, hModule, 0, NULL);
        if (hThread)
        {
            CloseHandle(hThread);
        }
    }
    return TRUE;
}
