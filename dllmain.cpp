// dllmain.cpp : Defines the entry point for the DLL application.
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <iostream>

#include "cheat.h"
#include "Classes.h"

#define RELOC_FLAG(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_HIGHLOW)
using f_DLL_ENTRY_POINT = BOOL(WINAPI*)(void* hDll, DWORD dwReason, void* pReserved);

void* CaptureInterface(LPCSTR strModule, LPCSTR strInterface) {
	typedef void* (*CreateInterfaceFn)(const char* szName, int iReturn);
	return ((CreateInterfaceFn)GetProcAddress(GetModuleHandleA(strModule), "CreateInterface"))(strInterface, 0);
}

BOOL MainThread(HMODULE hModule) {

	/*AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);

	HANDLE hProc = GetCurrentProcess();*/

	ICVar * CVar = (ICVar*)CaptureInterface("vstdlib.dll", "VEngineCvar007");
	IEngine* Engine = (IEngine*)CaptureInterface("engine.dll", "VEngineClient014");

	BYTE* pSrcData = dll_data;

	IMAGE_NT_HEADERS * pOldNtHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(pSrcData + reinterpret_cast<IMAGE_DOS_HEADER*>(pSrcData)->e_lfanew);
	IMAGE_OPTIONAL_HEADER* pOldOptHeader = &pOldNtHeader->OptionalHeader;
	IMAGE_FILE_HEADER* pOldFileHeader = &pOldNtHeader->FileHeader;

	BYTE* pTargetBase = (BYTE*)VirtualAlloc(0, pOldOptHeader->SizeOfImage, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	int error = GetLastError();
	//if (!pTargetBase) {
		//pTargetBase = reinterpret_cast<BYTE*>(VirtualAllocEx(hProc, nullptr, pOldOptHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
		if (!pTargetBase) {
			//printf("Memory allocation failed (ex) 0x%X\n", GetLastError());
			delete[] pSrcData;
			FreeLibraryAndExitThread(hModule, 0);
			return TRUE;
		}
	//}

	auto* pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
	for (UINT i = 0; i != pOldFileHeader->NumberOfSections; ++i, ++pSectionHeader) {
		if (pSectionHeader->SizeOfRawData) {
			memcpy(pTargetBase + pSectionHeader->VirtualAddress, pSrcData + pSectionHeader->PointerToRawData, pSectionHeader->SizeOfRawData);
			//printf("Can't map sections: 0x%x\n", GetLastError()); //sucess print
		}
	}
	memcpy(pTargetBase, pSrcData, 0x1000);

	auto* pOpt = &reinterpret_cast<IMAGE_NT_HEADERS*>(pTargetBase + reinterpret_cast<IMAGE_DOS_HEADER*>(pTargetBase)->e_lfanew)->OptionalHeader;
	auto _DllMain = reinterpret_cast<f_DLL_ENTRY_POINT>(pTargetBase + pOpt->AddressOfEntryPoint);

	BYTE* LocationDelta = pTargetBase - pOpt->ImageBase;
	if (LocationDelta) {
		if (!pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) {
			FreeLibraryAndExitThread(hModule, 0);
			return TRUE;
		}

		auto* pRelocData = reinterpret_cast<IMAGE_BASE_RELOCATION*>(pTargetBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
		while (pRelocData->VirtualAddress) {
			UINT AmountOfEntries = (pRelocData->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
			WORD* pRelativeInfo = reinterpret_cast<WORD*>(pRelocData + 1);

			for (UINT i = 0; i != AmountOfEntries; ++i, ++pRelativeInfo) {
				if (RELOC_FLAG(*pRelativeInfo)) {
					UINT_PTR* pPatch = reinterpret_cast<UINT_PTR*>(pTargetBase + pRelocData->VirtualAddress + ((*pRelativeInfo) & 0xFFF));
					*pPatch += reinterpret_cast<UINT_PTR>(LocationDelta);
				}
			}
			pRelocData = reinterpret_cast<IMAGE_BASE_RELOCATION*>(reinterpret_cast<BYTE*>(pRelocData) + pRelocData->SizeOfBlock);
		}
	}

	if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
		auto* pImportDescr = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(pTargetBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		while (pImportDescr->Name) {
			char* szMod = reinterpret_cast<char*>(pTargetBase + pImportDescr->Name);
			HINSTANCE hDll = LoadLibrary(szMod);

			ULONG_PTR* pThunkRef = reinterpret_cast<ULONG_PTR*>(pTargetBase + pImportDescr->OriginalFirstThunk);
			ULONG_PTR* pFuncRef = reinterpret_cast<ULONG_PTR*>(pTargetBase + pImportDescr->FirstThunk);

			if (!pThunkRef)
				pThunkRef = pFuncRef;

			for (; *pThunkRef; ++pThunkRef, ++pFuncRef) {
				if (IMAGE_SNAP_BY_ORDINAL(*pThunkRef)) {
					*pFuncRef = (ULONG_PTR)GetProcAddress(hDll, reinterpret_cast<char*>(*pThunkRef & 0xFFFF));
				} else {
					auto* pImport = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(pTargetBase + (*pThunkRef));
					*pFuncRef = (ULONG_PTR)GetProcAddress(hDll, pImport->Name);
				}
			}
			++pImportDescr;
		}
	}

	BYTE* magic = (BYTE*)VirtualAlloc(0, 50, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	*reinterpret_cast<DWORD*>(magic) = 1337;
	strcpy(reinterpret_cast<char*>(magic + sizeof(DWORD)), "dick");

	_DllMain(pTargetBase, DLL_PROCESS_ATTACH, reinterpret_cast<void*>(magic));

	Sleep(2500);

	Engine->ClientCmd_Unrestricted("toggleconsole", 0);

	const char * asci =
		"               _____                _            _  \x00"
		"              / ____|              | |          | | \x00"
		"             | |     _ __ __ _  ___| | _____  __| | \x00"
		"             | |    | '__/ _` |/ __| |/ / _ \\/ _` | \x00"
		"             | |____| | | (_| | (__|   <  __/ (_| | \x00"
		"              \\_____|_|  \\__,_|\\___|_|\\_\\___|\\__,_| \x00"
		" \x00"
		" _                 _                 _                   _____  _      \x00"
		"| |               | |               | |                 |  __ \\| |     \x00"
		"| |__  _   _    __| |_   _ _ __ ___ | |__   __ _ ___ ___| |__) | |     \x00"
		"| '_ \\| | | |  / _` | | | | '_ ` _ \\| '_ \\ / _` / __/ __|  ___/| |     \x00"
		"| |_) | |_| | | (_| | |_| | | | | | | |_) | (_| \\__ \\__ \\ |    | |____ \x00"
		"|_.__/ \\__, |  \\__,_|\\__,_|_| |_| |_|_.__/ \\__,_|___/___/_|    |______|\x00"
		"        __/ |                                                          \x00"
		"        |___/                                                          \x00"
		" \x00\x00";

	char* art = (char*)asci;

	float h = 0;
	while (*art != 0) {
		h = 0;
		for (size_t i = 0; i < strlen(art); i++) {
			char cha[2] = " ";
			cha[0] = *(art + i);
			CVar->ConsoleColorPrintf(Color::FromHSB(h, 1.f, 1.f), "%s", cha);
			h += 0.008f;
			if (h > 1.0) h = 0.f;
		}
		CVar->ConsoleColorPrintf(Color(0, 0, 0), "\n");
		art = art + strlen(art) + 1;
	}
	CVar->ConsoleColorPrintf(Color(0, 255, 0), "\n");

	CVar->ConsoleColorPrintf(Color(0, 255, 0), "donate(BTC): 37iE16RbjUt7L62LSxoJb2VCmKALK9kjet\n");
	CVar->ConsoleColorPrintf(Color(0, 255, 0), "free cp discord: https://discord.gg/akNNjtf\n");

	FreeLibraryAndExitThread(hModule, 0);

	return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, 0);
		//DisableThreadLibraryCalls(hModule);
	}
    return TRUE;
}

