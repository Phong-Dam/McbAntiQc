#include "Jass.h"
namespace Jass {
	pGetTriggerPlayer GetTriggerPlayer;
	pPlayer PlayerGame;
	bool IsOkayPtr(void* ptr, unsigned int size)
	{
		bool returnvalue = false;
		returnvalue = IsBadReadPtr(ptr, size) == 0;
		return returnvalue;
	}
	int Player(int number)
	{
		int arg1 = Game::GetGlobalPlayerData();
		int result = 0;

		if (number < 0 || number > 15)
		{
			return 0;
		}

		if (arg1 > NULL)
		{
			result = (int)arg1 + (number * 4) + 0x58;

			if (result)
			{
				result = *(int*)result;
			}
			else
			{
				return 0;
			}
		}
		return result;
	}
	int GetLocalPlayer()
	{
		int gldata = Game::GetGlobalPlayerData();
		if (gldata > 0)
		{
			short retval = *(short*)(gldata + 0x28);
			return retval;
		}
		return 0;
	}
	int GetLocalPlayerId()
	{
		int gldata = Game::GetGlobalPlayerData();
		if (gldata > 0)
		{
			int playerslotaddr = (int)gldata + 0x28;
			if (IsOkayPtr((void*)playerslotaddr))
				return (int)*(short*)(playerslotaddr);
			else
				return -2;
		}
		else
			return -2;
	}
	unit GetLocalSelect()
	{
		int plr = Player(GetLocalPlayer());
		if (plr > 0)
		{
			int PlayerData1 = *(int*)(plr + 0x34);
			if (PlayerData1 > 0)
			{
				return *(unsigned char**)(PlayerData1 + 0x1e0);
			}
		}
		return NULL;
	}
	CHAR* JtoChar(DWORD JSID)
	{
		DWORD Convert = GameDll + 0x8CC410;
		DWORD GetCurrentJassEnvironment = GameDll + 0x8BED60;
		CHAR* cRet;

		__asm
		{
			push JSID;
			mov ecx, 1;
			call GetCurrentJassEnvironment;
			mov ecx, eax;
			call Convert;
			mov ecx, dword ptr ds : [eax + 0x08] ;
			mov eax, dword ptr ds : [ecx + 0x1C] ;
			mov cRet, eax;
		}

		return cRet;
	}
	void Init() {
		PlayerGame = (pPlayer)(GameDll + 0x242080);
		GetTriggerPlayer = (pGetTriggerPlayer)(GameDll + 0x236020);
	}
}