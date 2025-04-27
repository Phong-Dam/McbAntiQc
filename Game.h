#pragma once
#include "Init.h"
#include <string>
typedef DWORD __fastcall GAME_GetPtrList_t(VOID);
extern GAME_GetPtrList_t* GAME_GetPtrList;
typedef void __fastcall GAME_Print_t(DWORD ptrList, DWORD _EDX, const CHAR* text, DWORD* color, DWORD stayUpTime, DWORD _1);
extern GAME_Print_t* GAME_Print;
namespace Game {
	extern const char* PlayerName[12];
	void Init();
	bool IsInGame();
	bool IsChat();
	bool IsWindowActive();
	void PrintText(const char* message, float stayUpTime = 1);
	void PrintTextFormat(const char* format, ...);
	void DisplayTextFormat(const char* format, ...);
	void DisplayText(char* szText, float fDuration);
	void Patch(DWORD dwBaseAddress, const char* szData, int iSize);
	int TimeGet();
	int GetGlobalPlayerData();
	void ShowMessageBox(const char* message);
	void PlantDetourCall(BYTE* bSource, BYTE* bDestination, int iLength);
	const char* GetKeyName(WPARAM vkCode);
	uint8_t readByteFromAddress(uintptr_t address);
}