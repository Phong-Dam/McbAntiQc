#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "MinHook.h"
#include <atomic>
typedef unsigned int    uint32;
typedef unsigned char* unit;
long long GetRealTicks();
extern std::atomic<bool> CheckQC;
extern boolean debug;