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
#include <atomic> 

boolean debug = FALSE;


const int CHECK_QC_NEW = 111114;
const int SUNDAY_CHECK_CODE = 111112;
const int ENABLE_QC_CODE = 100024;
const int DISABLE_QC_CODE = 100025;
const DWORD EXPECTED_WC3_VERSION = 7680;

std::atomic<bool> CheckQC = false;

typedef unsigned int(__cdecl* sub_6F242A20)(int* a1);
sub_6F242A20 sub_6F242A20_org = nullptr, sub_6F242A20_ptr = nullptr;

int isSunday() {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    return (localTime->tm_wday == 0) ? 1 : 0;
}

unsigned int sub_6F242A20_my(int* a1) {
    int result = sub_6F242A20_ptr(a1);
    if (result == CHECK_QC_NEW)
        return 1;
    if (result == SUNDAY_CHECK_CODE)
        return isSunday();
    if (result == ENABLE_QC_CODE) {
        CheckQC.store(true);
    }
    if (result == DISABLE_QC_CODE) {
        CheckQC.store(false);
    }
    return result;
}

std::chrono::steady_clock::time_point realticks1 = std::chrono::steady_clock::now();

long long GetRealTicks() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - realticks1).count();
}

static HHOOK hMouseHook = nullptr;

LRESULT CALLBACK mouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= HC_ACTION && Game::IsInGame()) {
        const MSLLHOOKSTRUCT* mouseData = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
        if ((mouseData->flags & LLMHF_INJECTED) && wParam == WM_LBUTTONDOWN) {
            if (CheckQC.load())
                return 1;
        }
    }
    return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}

DWORD WINAPI MouseHookThread(LPVOID lpParam) {
    HINSTANCE hInstance = (HINSTANCE)lpParam;
    hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, mouseProc, hInstance, 0);
    if (!hMouseHook) {
        MessageBox(nullptr, "Failed to set mouse hook", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    MSG msg;
    /*
    while (true) {

        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {

            Sleep(1000);
        }
    }
    */
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(hMouseHook);
    hMouseHook = nullptr;
    return 0;
}

DWORD WINAPI MonitorGameState(LPVOID lpParam) {
    HMODULE hModule = (HMODULE)lpParam;
    while (true) {
        if (Game::IsInGame()) {
            HANDLE hThread = CreateThread(nullptr, 0, MouseHookThread, hModule, 0, nullptr);
            if (*(int*)(GameDll + 0x0BAD40) != -109638679) {
                Input::ReHook();
            }
        }
        Sleep(10000);
    }
    return 0;
}

DWORD WarcraftVersion() {
    DWORD dwHandle = 0;
    DWORD dwLen = GetFileVersionInfoSize("Game.dll", &dwHandle);
    if (dwLen == 0) return 0;

    LPVOID lpData = new char[dwLen];
    if (!GetFileVersionInfo("Game.dll", dwHandle, dwLen, lpData)) {
        delete[] lpData;
        return 0;
    }

    LPBYTE lpBuffer = nullptr;
    UINT uLen = 0;
    if (!VerQueryValue(lpData, "\\", (LPVOID*)&lpBuffer, &uLen)) {
        delete[] lpData;
        return 0;
    }

    VS_FIXEDFILEINFO* version = (VS_FIXEDFILEINFO*)lpBuffer;
    DWORD ver = LOWORD(version->dwFileVersionLS);
    delete[] lpData;
    return ver;
}

HRESULT __stdcall DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
    static HANDLE hMonitorThread = nullptr;

    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        if (WarcraftVersion() == EXPECTED_WC3_VERSION) {
            MH_Initialize();
            Game::Init();
            Input::Init();
            Chat::Init();
            Jass::Init();
            GetEventPlayerChatString::Init();

            sub_6F242A20_org = (sub_6F242A20)(GameDll + 0x242A20);
            MH_CreateHook(sub_6F242A20_org, &sub_6F242A20_my, reinterpret_cast<void**>(&sub_6F242A20_ptr));
            MH_EnableHook(sub_6F242A20_org);

            hMonitorThread = CreateThread(nullptr, 0, MonitorGameState, hModule, 0, nullptr);
            if (!hMonitorThread) {
                MessageBox(nullptr, "Failed to create monitor thread", "Error", MB_OK | MB_ICONERROR);
            }
        }
        break;

    case DLL_PROCESS_DETACH:
        if (hMouseHook) UnhookWindowsHookEx(hMouseHook);
        if (hMonitorThread) CloseHandle(hMonitorThread);
        MH_Uninitialize();
        break;
    }
    return TRUE;
}