#pragma once
#include <windows.h>

#define HOOK_SIZE 6

class Hook {
public:
	DWORD HookFunction(void* adress, void* hookFunction, int size);
	bool Unhook();
private:
	BYTE originalData[HOOK_SIZE];
	bool isHooked = false;
	void* location = 0;
	void* coppiedBytesLocation = 0;
	void* hookFunctionLocation = 0;
};