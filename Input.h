#pragma once
#include "includes.h"
#include "Chat.h"
#include "Game.h"
namespace Input {
	void Init();
	unsigned long GetAFKTime();
	float GetTotalAverageClickTime();
	float Get5MinuteAverageClickTime();
	extern int qc_detect;
	typedef LRESULT(__fastcall* InputProc)(HWND hWnd, unsigned int Msg, WPARAM wParam, LPARAM lParam);
	extern InputProc InputProc_ptr;
	extern InputProc InputProc_org;
	void ReHook();
	void UnHook();
	void Hook();
}