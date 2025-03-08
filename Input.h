#pragma once
#include "includes.h"
#include "Chat.h"
#include "Game.h"
namespace Input {
	void Init();
	unsigned long GetAFKTime();
	float GetTotalAverageClickTime();
	float Get5MinuteAverageClickTime();
}