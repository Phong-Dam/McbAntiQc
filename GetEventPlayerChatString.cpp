#include "GetEventPlayerChatString.h"
#include "Input.h"
namespace GetEventPlayerChatString {
	typedef int(__fastcall* pGetEventPlayerChatString)();
	pGetEventPlayerChatString GetEventPlayerChatString_org;
	pGetEventPlayerChatString GetEventPlayerChatString_ptr;
	typedef int(__cdecl* sub_6F22FEE0)(int a1);
	sub_6F22FEE0 DisplayCineFilter_org, DisplayCineFilter_ptr;
	long long Event_Tick = 0;
	boolean disablelogin = false;
	boolean disableCine = false;
	char* toLowerCase(const char* input) {
		int length = std::strlen(input);
		char* result = new char[length + 1];
		for (int i = 0; i < length; ++i) {
			result[i] = std::tolower(static_cast<unsigned char>(input[i]));
		}
		result[length] = '\0';
		return result;
	}
	int ExtractInteger(const char* realStr) {
		int value = -1;
		sscanf(realStr, "@ms %d", &value);
		return value;
	}
	int __fastcall GetEventPlayerChatString_my()
	{
		int result;
		result = GetEventPlayerChatString_ptr();
		int hPlayer = Jass::GetTriggerPlayer();
		char* realStr = Jass::JtoChar(result);
		unsigned long tempAFK;
		int tempInt = -1;
		if (realStr == NULL)
			return result;
		realStr = toLowerCase(Jass::JtoChar(result));
		if (GetRealTicks() - Event_Tick > 10)
		{
			Event_Tick = GetRealTicks();
		}
		else
			return result;
		if (realStr && realStr[0] != '\0')
		{
			if (strstr(realStr, "-login") != nullptr) {
				disablelogin = true;
			}
			if (strstr(realStr, "-flogin") != nullptr) {
				disablelogin = true;
			}
			if (strcmp(realStr, "@whovirus") == 0) {
				Chat::SendChatFormat(true, "|c00FFFF00MCB Helper : |cff00ff00Run!|r");
			}
			if (strcmp(realStr, "@qcdetect") == 0) {
				//Chat::SendChatFormat(true, "|c00FFFF00Qc Detect : |cff00ff00%d!|r", Input::qc_detect);
			}
			if (strcmp(realStr, "@whoafk") == 0) {
				tempAFK = Input::GetAFKTime();
				if (tempAFK > 0 )
					Chat::SendChatFormat(true, "|cff00ff00Im afk : %lu seconds|r", tempAFK);
				else if (tempAFK == -1)
					Chat::SendChatFormat(true, "|cff00ff00Im afk!|r");
			}	
			if (strstr(realStr, "@ms ") != nullptr) {
				tempInt = ExtractInteger(realStr);
				if (tempInt != -1) {
					if (hPlayer == Jass::PlayerGame(tempInt)) {
						Chat::SendChatFormat(true,
							"|c00FFFF00Avg |r: %.1f ms | |c00FFFF005-min Avg |r: %.1f ms",
							Input::GetTotalAverageClickTime(),
							Input::Get5MinuteAverageClickTime()
						);
					}
				}
			}
		}
		if (hPlayer == Jass::PlayerGame(Jass::GetLocalPlayerId())) {
			if (strcmp(realStr, "-imgoff") == 0) {
				disableCine = true;
				Game::DisplayText("|cffff0000Image : Off!|r", 5);
			}
			if (strcmp(realStr, "-imgon") == 0) {
				disableCine = false;
				Game::DisplayText("|cff00ff00Image : On!|r", 5);
			}
			if (strcmp(realStr, "@myms") == 0)
				Chat::SendChatFormat(true,
					"|c00FFFF00Avg |r: %.1f ms | |c00FFFF005-min Avg |r: %.1f ms",
					Input::GetTotalAverageClickTime(),
					Input::Get5MinuteAverageClickTime()
				);
				
		}
		delete[] realStr;

		return result;
	}
	int __cdecl DisplayCineFilter_my(int a1)
	{
		if (disableCine)
			if (disablelogin == true) {
				disablelogin = false;
				return DisplayCineFilter_ptr(a1);
			}
			else
				return DisplayCineFilter_ptr(0);
		return DisplayCineFilter_ptr(a1);
	}
	void Init() {
		GetEventPlayerChatString_org = (pGetEventPlayerChatString)(GameDll + 0x232530);
		MH_CreateHook(GetEventPlayerChatString_org, &GetEventPlayerChatString_my, reinterpret_cast<void**>(&GetEventPlayerChatString_ptr));
		MH_EnableHook(GetEventPlayerChatString_org);
		DisplayCineFilter_org = (sub_6F22FEE0)(GameDll + 0x22FEE0);
		MH_CreateHook(DisplayCineFilter_org, &DisplayCineFilter_my, reinterpret_cast<void**>(&DisplayCineFilter_ptr));
		MH_EnableHook(DisplayCineFilter_org);
	}
}