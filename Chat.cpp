#include "Chat.h"
namespace Chat {
	long long SendChat_tick = 0;
	int UsingCustomChatTarget = false;
	int CustomChatTarget = 0;
	typedef int(__fastcall* pSetChatTargetUsers)(int chataddr, int ecx, int valtype);
	typedef int(__fastcall* pGameChatSetState)(unsigned char* chat, int unused, int IsOpened);
	typedef int(__fastcall* pGameChatSendMessage)(int GlobalGlueObjAddr, int zero, unsigned char* event_vtable);
	pGameChatSendMessage GameChatSendMessage_org, GameChatSendMessage_ptr;
	pGameChatSetState GameChatSetState;
	pSetChatTargetUsers pSetChatTargetUsers_org;
	pSetChatTargetUsers pSetChatTargetUsers_ptr;
	unsigned char* _GlobalGlueObj;
	unsigned char* _EventVtable = 0;
	unsigned char* GetChatOffset()
	{
		unsigned char* pclass = *(unsigned char**)(GameDll + 0xD326F0);
		if (pclass)
		{
			return *(unsigned char**)(pclass + 0x3FC);
		}

		return 0;
	}
	char* GetChatString()
	{
		unsigned char* pChatOffset = GetChatOffset();
		if (pChatOffset)
		{
			pChatOffset = *(unsigned char**)(pChatOffset + 0x1E0);
			if (pChatOffset)
			{
				pChatOffset = *(unsigned char**)(pChatOffset + 0x1E4);
				return (char*)pChatOffset;
			}
		}
		return 0;
	}
	void SendChat(const char* text, bool All) {
		unsigned char* ChatOffset = GetChatOffset();
		char* pChatString = GetChatString();
		BlockInput(true);
		if (GetRealTicks() - SendChat_tick > 100) {
			SendChat_tick = GetRealTicks();
			if (/**(int*)ChatFound*/Game::IsChat())
			{
				UsingCustomChatTarget = true;
				if (All)
				{
					CustomChatTarget = 0;
				}
				else
				{
					CustomChatTarget = 1;
				}
				/* Close chat */
				pChatString[0] = '\0';
				GameChatSetState(ChatOffset, 0, 0);

				/* Open chat */
				GameChatSetState(ChatOffset, 0, 1);

				/* Set message */
				sprintf_s(pChatString, 128, "%.128s", text);

				/* Send Event */
				GameChatSendMessage_org(*(int*)_GlobalGlueObj, 0, _EventVtable);

				UsingCustomChatTarget = false;
			}
			else
			{
				UsingCustomChatTarget = true;

				if (All)
				{
					CustomChatTarget = 0;
				}
				else
				{
					CustomChatTarget = 1;
				}

				/* Open chat */
				GameChatSetState(ChatOffset, 0, 1);

				/* Set message */
				sprintf_s(pChatString, 128, "%.128s", text);

				/* Send Event */
				GameChatSendMessage_org(*(int*)_GlobalGlueObj, 0, _EventVtable);

				UsingCustomChatTarget = false;

			}

		}
		BlockInput(false);
	}
	void SendChatFormat(bool all, const char* format, ...)
	{
		char str[8192] = { 0 };
		va_list args;
		va_start(args, format);
		vsprintf_s(str, sizeof(str), format, args);
		va_end(args);
		SendChat(str, all);
	}
	int __fastcall SetChatTargetUsers_my(int chataddr, int ecx, int valtype)
	{
		if (!UsingCustomChatTarget)
			return pSetChatTargetUsers_ptr(chataddr, ecx, valtype);
		else
			return pSetChatTargetUsers_ptr(chataddr, ecx, CustomChatTarget);
	}
	int __fastcall GameChatSendMessage_my(int GlobalGlueObjAddr, int zero, unsigned char* event_vtable) {
		return GameChatSendMessage_ptr(GlobalGlueObjAddr, zero, event_vtable);
	}
	void Init() {
		pSetChatTargetUsers_org = (pSetChatTargetUsers)(GameDll + 0x404600);
		MH_CreateHook(pSetChatTargetUsers_org, &SetChatTargetUsers_my, reinterpret_cast<void**>(&pSetChatTargetUsers_ptr));
		MH_EnableHook(pSetChatTargetUsers_org);
		GameChatSetState = (pGameChatSetState)(GameDll + 0x3E3890);
		GameChatSendMessage_org = (pGameChatSendMessage)(GameDll + 0x3A2E60);
		MH_CreateHook(GameChatSendMessage_org, &GameChatSendMessage_my, reinterpret_cast<void**>(&GameChatSendMessage_ptr));
		MH_EnableHook(GameChatSendMessage_org);
		_GlobalGlueObj = (unsigned char*)GameDll + 0xD0F600;
		_EventVtable = (unsigned char*)GameDll + 0xCB1B90;
	}
}