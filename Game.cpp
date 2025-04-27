#include "Game.h"
#include <string>
GAME_GetPtrList_t* GAME_GetPtrList = 0;
GAME_Print_t* GAME_Print = 0;
namespace Game {
	const char* PlayerName[12];
	typedef int(__stdcall* TimeGetFunc)();
	TimeGetFunc GetTime = (TimeGetFunc)(GameDll + 0x35EA10);
	void Init() {
		GAME_GetPtrList = (GAME_GetPtrList_t*)(GameDll + 0x11BD30);
		GAME_Print = (GAME_Print_t*)(GameDll + 0x144480);
	}
	bool IsInGame() {

		return *(int*)(GameDll + 0xD328D0) > 0 || *(int*)(GameDll + 0xD30110) > 0;
	}
	bool IsInGame2() {

		return Game::TimeGet() > 0;
	}
	bool IsWindowActive()
	{
		return *(bool*)(GameDll + 0xCA3E74);
	}
	int GetGlobalPlayerData()
	{
		return  *(int*)(0xD305E0 + GameDll);
	}
	void ShowMessageBox(const char* message) {
		MessageBoxA(NULL, message, "Test", MB_OK | MB_ICONINFORMATION);
	}
	void PrintText(const char* message, float stayUpTime)
	{
		if (IsInGame())
		{
			DWORD ptrList = GAME_GetPtrList();
			if (ptrList)
			{
				DWORD color = 0xFFFFFFFF;
				GAME_Print(*((DWORD*)(ptrList + 0x3EC)), ptrList, message, &color, *((DWORD*)&stayUpTime), NULL);
			}

		}
	}
	bool IsChat()
	{
		return*(bool*)(GameDll + 0xD04FEC);
	}
	int TimeGet() {
		return GetTime();
	}
	void PrintTextFormat(const char* format, ...)
	{
		if (IsInGame())
		{
			char str[8192] = { 0 };
			va_list args;
			va_start(args, format);
			vsprintf_s(str, sizeof(str), format, args);
			va_end(args);
			PrintText(str, 5);
		}
	}
	void DisplayText(char* szText, float fDuration)
	{
		unsigned char* pW3XGlobalClass = (unsigned char*)GameDll + 0xD326F0;
		unsigned int dwDuration = *((unsigned int*)&fDuration);
		if (!*(unsigned char**)pW3XGlobalClass)
			return;
		int GlobalClassOffset = *(int*)(pW3XGlobalClass);
		if (GlobalClassOffset > 0)
			GlobalClassOffset = *(int*)(GlobalClassOffset);
		typedef void(__fastcall* GAME_PrintToScreen_t)(int ecx, uint32_t edx, uint32_t arg1, uint32_t arg2, char* outLinePointer, uint32_t dwDuration, uint32_t arg5);
		int ecx = *(int*)pW3XGlobalClass;

		unsigned char* GAME_PrintToScreen = (unsigned char*)GameDll + 0x3A8EB0;
		GAME_PrintToScreen_t _GAME_PrintToScreen = (GAME_PrintToScreen_t)GAME_PrintToScreen;
		_GAME_PrintToScreen(ecx, 0x0, 0x0, 0x0, szText, dwDuration, 0xFFFFFFFF);
	}
	void DisplayTextFormat(const char* format, ...) {
		if (IsInGame())
		{
			char str[8192] = { 0 };
			va_list args;
			va_start(args, format);
			vsprintf_s(str, sizeof(str), format, args);
			va_end(args);
			DisplayText(str, 10);
		}
	}
	void PlantDetourCall(BYTE* bSource, BYTE* bDestination, int iLength)
	{
		DWORD dwOldProtection = NULL;

		BYTE* bJump = (BYTE*)malloc(iLength + 5);

		VirtualProtect(bSource, iLength, PAGE_EXECUTE_READWRITE, &dwOldProtection);
		memcpy(bJump + 3, bSource, iLength);

		bJump[0] = 0x58;
		bJump[1] = 0x59;
		bJump[2] = 0x50;
		bJump[iLength + 3] = 0xE9;
		*(DWORD*)(bJump + iLength + 4) = (DWORD)((bSource + iLength) - (bJump + iLength + 3)) - 5;

		bSource[0] = 0xE8;
		*(DWORD*)(bSource + 1) = (DWORD)(bDestination - (bSource)) - 5;

		for (int i = 5; i < iLength; i++)
			bSource[i] = 0x90;

		VirtualProtect(bSource, iLength, dwOldProtection, NULL);
	}
	void Patch(DWORD dwBaseAddress, const char* szData, int iSize)
	{
		DWORD dwOldProtection = NULL;
		VirtualProtect((LPVOID)dwBaseAddress, iSize, PAGE_EXECUTE_READWRITE, &dwOldProtection);
		CopyMemory((LPVOID)dwBaseAddress, szData, iSize);
		VirtualProtect((LPVOID)dwBaseAddress, iSize, dwOldProtection, NULL);
	}
    uint8_t readByteFromAddress(uintptr_t address) {
        return *reinterpret_cast<volatile const uint8_t*>(address);
    }
    const char* GetKeyName(WPARAM vkCode) {
        static char buffer[32];

        if (vkCode >= 0x41 && vkCode <= 0x5A) {
            buffer[0] = (char)vkCode;
            buffer[1] = '\0';
            return buffer;
        }
        if (vkCode >= 0x30 && vkCode <= 0x39) {
            buffer[0] = (char)vkCode;
            buffer[1] = '\0';
            return buffer;
        }

        switch (vkCode) {
        case VK_ESCAPE:        return "Esc";
        case VK_BACK:          return "Backspace";
        case VK_TAB:           return "Tab";
        case VK_RETURN:        return "Enter";
        case VK_SPACE:         return "Space";
        case VK_PRIOR:         return "Page Up";
        case VK_NEXT:          return "Page Down";
        case VK_END:           return "End";
        case VK_HOME:          return "Home";
        case VK_LEFT:          return "Left Arrow";
        case VK_UP:            return "Up Arrow";
        case VK_RIGHT:         return "Right Arrow";
        case VK_DOWN:          return "Down Arrow";
        case VK_INSERT:        return "Insert";
        case VK_DELETE:        return "Delete";
        case VK_HELP:          return "Help";
        case VK_LWIN:          return "Left Windows";
        case VK_RWIN:          return "Right Windows";
        case VK_APPS:          return "Applications";
        case VK_SLEEP:         return "Sleep";
        case VK_NUMPAD0:       return "Numpad 0";
        case VK_NUMPAD1:       return "Numpad 1";
        case VK_NUMPAD2:       return "Numpad 2";
        case VK_NUMPAD3:       return "Numpad 3";
        case VK_NUMPAD4:       return "Numpad 4";
        case VK_NUMPAD5:       return "Numpad 5";
        case VK_NUMPAD6:       return "Numpad 6";
        case VK_NUMPAD7:       return "Numpad 7";
        case VK_NUMPAD8:       return "Numpad 8";
        case VK_NUMPAD9:       return "Numpad 9";
        case VK_MULTIPLY:      return "Numpad *";
        case VK_ADD:           return "Numpad +";
        case VK_SEPARATOR:     return "Separator";
        case VK_SUBTRACT:      return "Numpad -";
        case VK_DECIMAL:       return "Numpad .";
        case VK_DIVIDE:        return "Numpad /";
        case VK_F1:            return "F1";
        case VK_F2:            return "F2";
        case VK_F3:            return "F3";
        case VK_F4:            return "F4";
        case VK_F5:            return "F5";
        case VK_F6:            return "F6";
        case VK_F7:            return "F7";
        case VK_F8:            return "F8";
        case VK_F9:            return "F9";
        case VK_F10:           return "F10";
        case VK_F11:           return "F11";
        case VK_F12:           return "F12";
        case VK_F13:           return "F13";
        case VK_F14:           return "F14";
        case VK_F15:           return "F15";
        case VK_F16:           return "F16";
        case VK_F17:           return "F17";
        case VK_F18:           return "F18";
        case VK_F19:           return "F19";
        case VK_F20:           return "F20";
        case VK_F21:           return "F21";
        case VK_F22:           return "F22";
        case VK_F23:           return "F23";
        case VK_F24:           return "F24";
        case VK_SHIFT:         return "Shift";
        case VK_CONTROL:       return "Ctrl";
        case VK_MENU:          return "Alt";
        case VK_CAPITAL:       return "Caps Lock";
        case VK_NUMLOCK:       return "Num Lock";
        case VK_SCROLL:        return "Scroll Lock";
        case VK_OEM_1:         return "OEM_1 (;)";
        case VK_OEM_PLUS:      return "OEM_PLUS (=)";
        case VK_OEM_COMMA:     return "OEM_COMMA (,)";
        case VK_OEM_MINUS:     return "OEM_MINUS (-)";
        case VK_OEM_PERIOD:    return "OEM_PERIOD (.)";
        case VK_OEM_2:         return "OEM_2 (/)";
        case VK_OEM_3:         return "OEM_3 (`)";
        case VK_OEM_4:         return "OEM_4 ([)";
        case VK_OEM_5:         return "OEM_5 (\\)";
        case VK_OEM_6:         return "OEM_6 (])";
        case VK_OEM_7:         return "OEM_7 (')";
        case VK_PRINT:         return "Print";
        case VK_SNAPSHOT:      return "Print Screen";
        case VK_CANCEL:        return "Break";
        case VK_PROCESSKEY:    return "Process Key";
        case VK_PACKET:        return "Packet";
        case VK_ATTN:          return "Attn";
        case VK_CRSEL:         return "CrSel";
        case VK_EXSEL:         return "ExSel";
        case VK_EREOF:         return "Erase EOF";
        case VK_PLAY:          return "Play";
        case VK_ZOOM:          return "Zoom";
        case VK_NONCONVERT:    return "Nonconvert";
        case VK_OEM_CLEAR:     return "Clear";
        default:
            snprintf(buffer, sizeof(buffer), "Key_0x%X", vkCode);
            return buffer;
        }
    }
}