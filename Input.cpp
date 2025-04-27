#include "Input.h"
#include <string>
#include "Jass.h"
#include <vector> 
#include <deque>
#include <thread>
#include <mutex>
#include <optional>
#include <atomic> 

namespace Input {
    HWND WARHWND = NULL;
    bool INGAME = false;
    std::optional<unsigned long> current_tick;
    const unsigned long AFK_THRESHOLD = 10000;
    int quickClickDetectionEnabled = 0;
    InputProc InputProc_ptr;
    InputProc InputProc_org;

    static std::optional<unsigned long> last_key_time;
    static WPARAM last_key = 0;
    static std::optional<unsigned long> last_lbuttondown_time;
    std::deque<bool> qc_history;
    bool isQuickClickDetected = false;
    int qc_count = 0;

    struct ClickRecord {
        unsigned long delta;
        long long timestamp;
    };
    static std::vector<ClickRecord> clickHistory;
    const size_t MAX_CLICK_HISTORY = 1000;

    std::mutex mtx;

    
    static std::atomic<bool> simulating = false;

    bool IsAFK() {
        if (!current_tick.has_value()) return true;
        unsigned long current = GetRealTicks();
        return (current - current_tick.value()) > AFK_THRESHOLD;
    }

    unsigned long GetAFKTime() {
        if (!current_tick.has_value()) {
            return 0; // Hoặc giá trị phù hợp nếu người dùng chưa từng active
        }
        unsigned long current = GetRealTicks();
        unsigned long afk_duration = current - current_tick.value();
        if (afk_duration > AFK_THRESHOLD) {
            return afk_duration / 1000;
        }
        return 0;
    }

    float GetTotalAverageClickTime() {
        std::lock_guard<std::mutex> lock(mtx);
        if (clickHistory.empty()) return 0.0f;

        unsigned long long total = 0;
        for (const auto& record : clickHistory) {
            total += record.delta;
        }
        return static_cast<float>(total) / clickHistory.size();
    }

    float Get5MinuteAverageClickTime() {
        std::lock_guard<std::mutex> lock(mtx);
        unsigned long current = GetRealTicks();
        unsigned long cutoff = (current > 300000) ? current - 300000 : 0;
        unsigned long long total = 0;
        int count = 0;
        for (const auto& record : clickHistory) {
            if (record.timestamp >= cutoff) {
                total += record.delta;
                count++;
            }
        }
        return count > 0 ? static_cast<float>(total) / count : 0.0f;
    }

    void War3ClickMouse() {
        if (simulating.load()) return; 
        simulating.store(true);       

        POINT cursorhwnd;
        GetCursorPos(&cursorhwnd);
        ScreenToClient(WARHWND, &cursorhwnd);

        InputProc_ptr(WARHWND, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(cursorhwnd.x, cursorhwnd.y));
        InputProc_ptr(WARHWND, WM_LBUTTONUP, 0, MAKELPARAM(cursorhwnd.x, cursorhwnd.y));

        simulating.store(false);      
    }

    LRESULT __fastcall Input(HWND hWnd, unsigned int _Msg, WPARAM _wParam, LPARAM lParam)
    {
        if (simulating.load()) {      
            return InputProc_ptr(hWnd, _Msg, _wParam, lParam);
        }

        unsigned int Msg = _Msg;
        WPARAM wParam = _wParam;
        WARHWND = hWnd;

        if (Game::IsInGame() && Game::IsWindowActive() && !Game::IsChat() && Jass::GetLocalSelect() != NULL) {
            current_tick = GetRealTicks();
        }

        
        if (Game::IsInGame()) {
            if (!INGAME) {
                INGAME = true;
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    clickHistory.clear();
                }
                quickClickDetectionEnabled = 0;
                isQuickClickDetected = false;
            }
        }
        else {
            if (INGAME) {
                INGAME = false;
            }
        }

        if (!CheckQC.load()) {
            return InputProc_ptr(hWnd, Msg, wParam, lParam);
        }

        if (Game::IsInGame() && Game::IsWindowActive() && !Game::IsChat() && Jass::GetLocalSelect() != NULL) {
            if (_Msg == WM_KEYDOWN || _Msg == WM_SYSKEYDOWN) {
                if ((lParam & 0x40000000) == 0) {
                    last_key = wParam;
                    last_key_time = GetRealTicks();
                }
                else {
                    return DefWindowProc(hWnd, _Msg, _wParam, lParam);
                }
            }
            else if (_Msg == WM_LBUTTONDOWN) {
                last_lbuttondown_time = GetRealTicks();
                if (last_key_time.has_value()) {
                    unsigned long delta = GetRealTicks() - last_key_time.value();
                    const char* key_name = Game::GetKeyName(last_key);

                    if (delta < 100) {
                        {
                            std::lock_guard<std::mutex> lock(mtx);
                            clickHistory.push_back(ClickRecord{ delta, GetRealTicks() });
                            if (clickHistory.size() > MAX_CLICK_HISTORY) {
                                clickHistory.erase(clickHistory.begin());
                            }
                        }
                        Game::PrintTextFormat("|c00FFFF00%s|r : %lu ms", key_name, delta);
                    }
                    if (isQuickClickDetected) {
                        return DefWindowProc(hWnd, _Msg, _wParam, lParam);
                    }
                    last_key_time.reset();
                    last_key = 0;
                }
            }
            else if (_Msg == WM_LBUTTONUP) {
                if (last_lbuttondown_time.has_value()) {
                    unsigned long delta = GetRealTicks() - last_lbuttondown_time.value();
                    if (debug)
                        Game::PrintTextFormat("|c00FFFF00Time|r : %lu ms", delta);
                    bool isQC = (delta <= 20);
                    if (isQuickClickDetected) {
                        if (isQC) {
                            if (debug)
                                Game::PrintTextFormat("Cancel Click");
                            return DefWindowProc(hWnd, _Msg, _wParam, lParam);
                        }
                        else {
                            if (debug)
                                Game::PrintTextFormat("Game Click");
                            War3ClickMouse();
                        }
                    }

                    {
                        std::lock_guard<std::mutex> lock(mtx);
                        qc_history.push_back(isQC);
                        if (isQC) qc_count++;
                        if (qc_history.size() > 10) {
                            if (qc_history.front()) qc_count--;
                            qc_history.pop_front();
                        }
                        isQuickClickDetected = (qc_count >= 3);
                    }

                    last_lbuttondown_time.reset();
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
    void ReHook() {
        MH_DisableHook(InputProc_org);
        MH_RemoveHook(InputProc_org);
        MH_CreateHook(InputProc_org, &Input, reinterpret_cast<void**>(&InputProc_ptr));
        MH_EnableHook(InputProc_org);
    }
    void UnHook() {
        MH_DisableHook(InputProc_org);
        MH_RemoveHook(InputProc_org);
    }
    void Hook() {
        MH_CreateHook(InputProc_org, &Input, reinterpret_cast<void**>(&InputProc_ptr));
        MH_EnableHook(InputProc_org);
    }
}