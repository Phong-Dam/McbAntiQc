#pragma once
#include "includes.h"
#include "Game.h"
namespace Chat {
	void SendChat(const char* text, bool All);
	void SendChatFormat(bool all, const char* format, ...);
	void Init();
}