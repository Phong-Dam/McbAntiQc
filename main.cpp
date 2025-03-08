#define CINTERFACE
#include <Windows.h>
#include "includes.h"
#include "Init.h"
#include "Game.h"
#include "Chat.h"
#include "Input.h"
#include "GetEventPlayerChatString.h"
#include <chrono>
#include <ctime>

typedef unsigned int(__cdecl* sub_6F242A20)(int* a1);
sub_6F242A20 sub_6F242A20_org, sub_6F242A20_ptr = NULL;

int isSunday() {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    return (localTime->tm_wday == 0) ? 1 : 0;
}
unsigned int __cdecl sub_6F242A20_my(int* a1)
{
    int i = sub_6F242A20_ptr(a1);
    if (i == 111111)
        return 1;
    if (i == 111112)
        return isSunday();
    return i;
}
std::chrono::steady_clock::time_point realticks1 = std::chrono::steady_clock::now();

long long GetRealTicks()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - realticks1).count();
}
static HHOOK hMouseHook = NULL;
LRESULT CALLBACK mouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= HC_ACTION && Game::IsInGame())
    {
        const MSLLHOOKSTRUCT* mouseData = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);

        if ((mouseData->flags & LLMHF_INJECTED) && wParam == WM_LBUTTONDOWN)
        {
            return 1;
        }
    }

    return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}

DWORD WINAPI MouseHookThread(LPVOID lpParam)
{
    HINSTANCE hInstance = GetModuleHandle(NULL);

    hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, mouseProc, hInstance, 0);
    if (!hMouseHook) {
        return 1;
    }
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hMouseHook);
    hMouseHook = NULL;
    return 0;
}

HRESULT __stdcall DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    static HANDLE hThread = NULL;

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        MH_Initialize();
        Game::Init();
        Input::Init();
        Chat::Init();
        Jass::Init();
        GetEventPlayerChatString::Init();
        sub_6F242A20_org = (sub_6F242A20)(GameDll + 0x242A20);
        MH_CreateHook(sub_6F242A20_org, &sub_6F242A20_my, reinterpret_cast<void**>(&sub_6F242A20_ptr));
        MH_EnableHook(sub_6F242A20_org);
        hThread = CreateThread(NULL, 0, MouseHookThread, NULL, 0, NULL);
        if (!hThread) {
        }
        break;

    case DLL_PROCESS_DETACH:
        if (hMouseHook) {
            UnhookWindowsHookEx(hMouseHook);
        }
        if (hThread) {
            CloseHandle(hThread);
        }
        MH_Uninitialize();
        break;
    }

    return TRUE;
}