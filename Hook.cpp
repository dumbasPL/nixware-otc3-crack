#include "Hook.h"

DWORD Hook::HookFunction(void* adress, void* hookFunction, int size) {
	if (size < HOOK_SIZE) return NULL;
	if (isHooked) Unhook();
	hookFunctionLocation = hookFunction;
	location = adress;
	BYTE jmp[HOOK_SIZE] = {
		0x68, //push
		0x00, 0x00, 0x00, 0x00,  //address
		0xC3 //ret
	};
	DWORD offset2 = (DWORD)location + size;
	memcpy(&jmp[1], &hookFunctionLocation, 4);
	DWORD dwProtect;
	if (VirtualProtect(location, HOOK_SIZE, PAGE_EXECUTE_READWRITE, &dwProtect)) {
		memcpy(originalData, location, HOOK_SIZE);
		coppiedBytesLocation = VirtualAlloc(0, size + 6, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		memcpy(coppiedBytesLocation, location, size);
		*(BYTE*)((DWORD)coppiedBytesLocation + size) = 0x68;
		memcpy((void*)((DWORD)coppiedBytesLocation + size + 1), &offset2, 4);
		*(BYTE*)((DWORD)coppiedBytesLocation + size + 5) = 0xC3;
		memcpy((LPVOID)location, jmp, HOOK_SIZE);
		VirtualProtect(location, HOOK_SIZE, dwProtect, NULL);
		VirtualProtect(coppiedBytesLocation, size + 6, PAGE_EXECUTE_READ, NULL);
		isHooked = true;
		return (DWORD)coppiedBytesLocation;
	}
	return NULL;
}

bool Hook::Unhook() {
	if (!isHooked) return false;
	DWORD dwProtect;
	if (VirtualProtect(location, HOOK_SIZE, PAGE_EXECUTE_READWRITE, &dwProtect)) {
		memcpy((LPVOID)location, originalData, HOOK_SIZE);
		VirtualProtect(location, HOOK_SIZE, dwProtect, NULL);
		//VirtualFree(coppiedBytesLocation, 0, MEM_RELEASE);
		isHooked = false;
		return true;
	}
	return false;

}