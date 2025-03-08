#pragma once
#include "includes.h"
#include "Game.h"
#include "Chat.h"
namespace Jass {
	typedef int(__cdecl* pGetTriggerPlayer)();
	extern pGetTriggerPlayer GetTriggerPlayer;
	typedef int(__cdecl* pPlayer)(int number);
	extern pPlayer PlayerGame;
	unit GetLocalSelect();
	CHAR* JtoChar(DWORD JSID);
	int GetLocalPlayer();
	int GetLocalPlayerId();
	bool IsOkayPtr(void* ptr, unsigned int size = 4);
	void Init();
}