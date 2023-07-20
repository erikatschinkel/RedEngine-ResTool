#include <Windows.h>
#include <memory>
#include <stdio.h>
#include "../CP2077Res/MinHook.h"

#pragma comment(lib, "../CP2077Res/libMinHook.x64.lib")

static const char* g_gameName = "The Witcher 3";
static const char* g_moduleName = "witcher3.exe";
static const char* g_className = "W2ViewportClass";
static const char* g_windowTitle = "The Witcher 3";

HINSTANCE g_gameHandle = NULL;
HINSTANCE g_dllHandle = NULL;
HWND g_gameHwnd = NULL;

bool g_shutdown = false;

MH_STATUS status;

typedef __int64(__fastcall* tUpdateRes)(__int64, float);

tUpdateRes oUpdateRes = nullptr;

__int64 __fastcall hUpdateRes(__int64 a1, float a2) {

    __int64* resolutionX = (__int64*)((*(__int64*)(*(__int64*)(a1 + 0x8))) + 0x1CC);
    __int64* resolutionY = (__int64*)((*(__int64*)(*(__int64*)(a1 + 0x8))) + 0x1D0);

    RECT gameWindow;
    ::GetClientRect(g_gameHwnd, &gameWindow);

    if (WM_SIZE && WM_EXITSIZEMOVE && ((gameWindow.right != *resolutionX) || (gameWindow.bottom != *resolutionY))) {

        *resolutionX = gameWindow.right;
        *resolutionY = gameWindow.bottom;
    }

    return oUpdateRes(a1, a2);
}

void Hook() {

    status = MH_Initialize();
    if (status != MH_OK)
        printf("Failed to initialize MinHook, MH_STATUS 0x%X\n", status);

    __int64 UpdateRes = (__int64)g_gameHandle + 0x323B28;
    MH_CreateHook((LPVOID)(UpdateRes), (LPVOID)(hUpdateRes), (LPVOID*)(&oUpdateRes));
    MH_EnableHook((LPVOID)(UpdateRes));

}



bool Initialize() {

    AllocConsole();
    FILE* pOut;
    freopen_s(&pOut, "CONOUT$", "w", stdout);
    printf("The Witcher 3 Res Tool Console Initialized\n");

    g_gameHwnd = FindWindowA(g_className, NULL);
    if (g_gameHwnd == NULL)
    {
        g_gameHwnd = FindWindowA(NULL, g_windowTitle);
        if (g_gameHwnd == NULL)
        {
            printf("Failed to retrieve window handle, GetLastError 0x%X\n", GetLastError());
            return false;
        }
    }

    g_gameHandle = ::GetModuleHandleA(g_moduleName);
    if (g_gameHandle == NULL)
    {
        printf("Failed to retrieve module handle, GetLastError 0x%X\n", GetLastError());
        return false;
    }

    Hook();
    if (status != MH_OK) {

        printf("Hook Failed, GetLastError 0x%X\n", GetLastError());
        return false;
    }

    return true;
}


DWORD WINAPI RunCT(LPVOID lpArg)
{
    Initialize();

    return 0;
}

DWORD WINAPI DllMain(_In_ HINSTANCE hInstance, _In_ DWORD fdwReason, _In_ LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        g_dllHandle = hInstance;
        CreateThread(NULL, NULL, RunCT, NULL, NULL, NULL);
    }

    return 1;
}

