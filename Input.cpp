#include "Input.h"
#include <string>
#include "Jass.h"
#include <vector>

namespace Input {
    HWND WARHWND = NULL;
    boolean INGAME = FALSE;
    unsigned long current_tick = -1;
    unsigned long AFK_THRESHOLD = 10000;

    typedef LRESULT(__fastcall* InputProc)(HWND hWnd, unsigned int Msg, WPARAM wParam, LPARAM lParam);
    InputProc InputProc_ptr;
    InputProc InputProc_org;

    // Biến lưu thông tin phím cuối cùng
    static unsigned long last_key_time = 0;
    static WPARAM last_key = 0;

    // Click statistics
    struct ClickRecord {
        unsigned long delta;
        unsigned long timestamp;
    };
    static std::vector<ClickRecord> clickHistory;

    // AFK functions
    bool IsAFK() {
        unsigned long current = GetRealTicks();
        return (current - current_tick) > AFK_THRESHOLD;
    }

    unsigned long GetAFKTime() {
        if (IsAFK()) {
            unsigned long current = GetRealTicks();
            return (current - current_tick) / 1000;
        }
        return current_tick == -1 ? -1 : 0;
    }

    // New average functions
    float GetTotalAverageClickTime() {
        if (clickHistory.empty()) return 0.0f;

        unsigned long long total = 0;
        for (const auto& record : clickHistory) {
            total += record.delta;
        }
        return static_cast<float>(total) / clickHistory.size();
    }
    float Get5MinuteAverageClickTime() {
        unsigned long current = GetRealTicks();
        unsigned long cutoff;
        unsigned long long total = 0;
        int count = 0;
        if (current > 300000)
            cutoff = current - 300000;
        else
            cutoff = 0;
        for (const auto& record : clickHistory) {
            if (record.timestamp >= cutoff) {
                total += record.delta;
                count++;
            }
        }

        return count > 0 ? static_cast<float>(total) / count : 0.0f;
    }

    LRESULT __fastcall Input(HWND hWnd, unsigned int _Msg, WPARAM _wParam, LPARAM lParam)
    {
        unsigned int Msg = _Msg;
        WPARAM wParam = _wParam;
        WARHWND = hWnd;
        current_tick = GetRealTicks();
        if (Game::IsInGame())
        {
            if (INGAME == FALSE)
            {
                INGAME = TRUE;
                clickHistory.clear();
            }
        }
        else
        {
            if (INGAME == TRUE)
            {
                INGAME = FALSE;
            }

        }
        if (Game::IsInGame() && Game::IsWindowActive() && !Game::IsChat() && Jass::GetLocalSelect() != NULL)
        {
            if (_Msg == WM_KEYDOWN || _Msg == WM_SYSKEYDOWN)
            {
                last_key = wParam;
                last_key_time = current_tick;
            }
            else if (_Msg == WM_LBUTTONDOWN)
            {
                if (last_key_time > 0)
                {
                    unsigned long delta = current_tick - last_key_time;
                    const char* key_name = Game::GetKeyName(last_key);

                    if (delta < 100) {
                        clickHistory.push_back({ delta, current_tick });
                        Game::PrintTextFormat("|c00FFFF00%s|r : %lu ms", key_name, delta);
                    }

                    last_key_time = 0;
                    last_key = 0;
                }
            }
        }

        return InputProc_ptr(hWnd, Msg, wParam, lParam);
    }

    void Init() {
        InputProc_org = (InputProc)(GameDll + 0x0BAD40);
        MH_CreateHook(InputProc_org, &Input, reinterpret_cast<void**>(&InputProc_ptr));
        MH_EnableHook(InputProc_org);
    }
}