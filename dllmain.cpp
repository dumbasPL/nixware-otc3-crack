// dllmain.cpp : Defines the entry point for the DLL application.

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
#include <windows.h>
#include <psapi.h>


#include "cheat.h"
#include "Classes.h"
#include "XorCompileTime.hpp"
#include "Hook.h"

#define RELOC_FLAG(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_HIGHLOW)
using f_DLL_ENTRY_POINT = BOOL(WINAPI*)(void* hDll, DWORD dwReason, void* pReserved);

void* CaptureInterface(LPCSTR strModule, LPCSTR strInterface) {
	typedef void* (*CreateInterfaceFn)(const char* szName, int iReturn);
	return ((CreateInterfaceFn)GetProcAddress(GetModuleHandleA(strModule), "CreateInterface"))(strInterface, 0);
}

unsigned char recv_1[] = {
	0x37, 0x64, 0x64, 0x30, 0x33, 0x64, 0x63, 0x36, 0x37, 0x33, 0x65, 0x31, 0x66, 0x37, 0x37, 0x37,
	0x62, 0x31, 0x36, 0x37, 0x64, 0x35, 0x64, 0x39, 0x31, 0x33, 0x39, 0x66, 0x36, 0x36, 0x37, 0x30
};

unsigned char send_1[] = {
	0x77, 0x31, 0x71, 0x7a, 0x67, 0x31, 0x77, 0x5a, 0x56, 0x72, 0x4f, 0x30, 0x39, 0x50, 0x4d, 0x77,
	0x47, 0x44, 0x38, 0x48, 0x2b, 0x62, 0x4a, 0x39, 0x36, 0x34, 0x32, 0x5a, 0x46, 0x36, 0x35, 0x79,
	0x37, 0x33, 0x35, 0x61, 0x65, 0x56, 0x30, 0x31, 0x77, 0x6f, 0x55, 0x33, 0x31, 0x4f, 0x38, 0x37,
	0x31, 0x31, 0x78, 0x50, 0x58, 0x63, 0x66, 0x46, 0x65, 0x76, 0x37, 0x59, 0x78, 0x71, 0x4b, 0x4c,
	0x6a, 0x61, 0x50, 0x2b, 0x68, 0x36, 0x49, 0x35, 0x53, 0x75, 0x48, 0x43, 0x64, 0x69, 0x54, 0x66,
	0x30, 0x72, 0x51, 0x72, 0x30, 0x52, 0x78, 0x44, 0x4c, 0x73, 0x4e, 0x51, 0x34, 0x67, 0x54, 0x35,
	0x73, 0x68, 0x5a, 0x46, 0x58, 0x65, 0x46, 0x51, 0x55, 0x41, 0x6b, 0x71, 0x59, 0x51, 0x2b, 0x51,
	0x54, 0x64, 0x75, 0x71, 0x37, 0x31, 0x2f, 0x79, 0x48, 0x44, 0x63, 0x67, 0x34, 0x37, 0x33, 0x55
};

unsigned char recv_2[] = {
	0x59, 0x74, 0x53, 0x67, 0x36, 0x67, 0x45, 0x79, 0x63, 0x54, 0x32, 0x69, 0x5a, 0x6a, 0x31, 0x63,
	0x6c, 0x65, 0x49, 0x4a, 0x76, 0x53, 0x72, 0x6d, 0x61, 0x4c, 0x48, 0x4b, 0x74, 0x6a, 0x43, 0x70,
	0x78, 0x6c, 0x33, 0x2f, 0x2b, 0x43, 0x53, 0x72, 0x4c, 0x67, 0x67, 0x3d
};

void hexDump(void* addr, int len) {
	int i;
	unsigned char buff[17];
	unsigned char* pc = (unsigned char*)addr;
	if (len == 0) {
		printf("  ZERO LENGTH\n");
		return;
	}
	if (len < 0) {
		printf("  NEGATIVE LENGTH: %i\n", len);
		return;
	}
	for (i = 0; i < len; i++) {
		if ((i % 16) == 0) {
			if (i != 0) printf("  %s\n", buff);
			printf("  %04x ", i);
		}
		printf(" %02x", pc[i]);
		if ((pc[i] < 0x20) || (pc[i] > 0x7e)) buff[i % 16] = '.';
		else buff[i % 16] = pc[i];
		buff[(i % 16) + 1] = '\0';
	}
	while ((i % 16) != 0) {
		printf("   ");
		i++;
	}
	printf("  %s\n", buff);
}

typedef int (WSAAPI* MySend)(SOCKET, char*, int, int);
typedef int (WINAPI* MyRecv)(SOCKET, char*, int, int);
typedef int (WSAAPI* MygetaddrinfoDetour)(PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA* pHints, PADDRINFOA* ppResult);

MySend OrigSend = NULL;
MyRecv OrigRecv = NULL;
MygetaddrinfoDetour OrigGetaddrinfoDetour = NULL;

int WSAAPI SendDetour(SOCKET s, char* buf, int len, int flags) {
	/*struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	getpeername(s, (struct sockaddr*) &addr, &addrlen);*/

	printf("send");
	hexDump(buf, len);
	return OrigSend(s, buf, len, flags);
}

int WINAPI RecvDetour(SOCKET s, char* buf, int len, int flags) {
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	getpeername(s, (struct sockaddr*) & addr, &addrlen);
	if (ntohs(addr.sin_port) == 8505) {
		printf("rekt\n");
		memcpy(buf, recv_1, sizeof(recv_1));
		return sizeof(recv_1);
	}

	int ret = OrigRecv(s, buf, len, flags);
	printf("recv");
	hexDump(buf, ret);
	return ret;
}

int WSAAPI getaddrinfoDetour(PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA* pHints, PADDRINFOA* ppResult) {
	printf("adr: %s\n", pNodeName);
	return OrigGetaddrinfoDetour(pNodeName, pServiceName, pHints, ppResult);
}

Hook send_hook;
Hook recv_hook;
Hook GetaddrinfoDetour_hook;

#define INRANGE(x,a,b)    (x >= a && x <= b)
#define getBits( x )    (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define getByte( x )    (getBits(x[0]) << 4 | getBits(x[1]))

DWORD FindPattern(std::string moduleName, std::string pattern) {
	const char* pat = pattern.c_str();
	DWORD firstMatch = 0;
	DWORD rangeStart = (DWORD)GetModuleHandleA(moduleName.c_str());
	MODULEINFO miModInfo;
	GetModuleInformation(GetCurrentProcess(), (HMODULE)rangeStart, &miModInfo, sizeof(MODULEINFO));
	DWORD rangeEnd = rangeStart + miModInfo.SizeOfImage;
	for (DWORD pCur = rangeStart; pCur < rangeEnd; pCur++) {
		if (!*pat) return firstMatch;
		if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == getByte(pat)) {
			if (!firstMatch) firstMatch = pCur;
			if (!pat[2]) return firstMatch;
			if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?') pat += 3;
			else pat += 2;
		} else {
			pat = pattern.c_str();
			firstMatch = 0;
		}
	}
	return NULL;
}

//BOOL UnloadThread(HMODULE hModule) {
//	Sleep(100);
//	FreeLibraryAndExitThread(hModule, 0);
//}
typedef void(__fastcall* ClanTagFn)(const char*, const char*);

ClanTagFn origSetClanTag = 0;
Hook clantag_h;

char* lastClanTag;

void SetClanHook(const char* tag, const char* name) {
	origSetClanTag("test", "test");
}

BOOL MainThread(HMODULE hModule) {

	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);

	printf("loaded\n");

	printf("send address: %X\n", (DWORD)&send);
	printf("recv address: %X\n", (DWORD)&recv);
	printf("Getaddrinfo address: %X\n", (DWORD)&getaddrinfo);

	system("pause");

	OrigSend = (MySend)send_hook.HookFunction((void*)&send, &SendDetour, 8);
	OrigRecv = (MyRecv)recv_hook.HookFunction((void*)&recv, &RecvDetour, 8);
	OrigGetaddrinfoDetour = (MygetaddrinfoDetour)GetaddrinfoDetour_hook.HookFunction((void*)&getaddrinfo, &getaddrinfoDetour, 8);

	//HANDLE hProc = GetCurrentProcess();

	ICVar * CVar = (ICVar*)CaptureInterface(_("vstdlib.dll"), _("VEngineCvar007"));
	IEngine* Engine = (IEngine*)CaptureInterface(_("engine.dll"), _("VEngineClient014"));

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
	strcpy(reinterpret_cast<char*>(magic + sizeof(DWORD)), _("dick"));

	_DllMain(pTargetBase, DLL_PROCESS_ATTACH, reinterpret_cast<void*>(magic));

	Sleep(2500);

	Engine->ClientCmd_Unrestricted(_("toggleconsole"), 0);

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

	CVar->ConsoleColorPrintf(Color(0, 255, 0), _("donate(BTC): 37iE16RbjUt7L62LSxoJb2VCmKALK9kjet\n"));
	CVar->ConsoleColorPrintf(Color(0, 255, 0), _("free cp discord: https://discord.gg/akNNjtf\n"));


	//DWORD SetClanTag = FindPattern("engine.dll", "53 56 57 8B DA 8B F9 FF 15");

	//origSetClanTag = (ClanTagFn)clantag_h.HookFunction((void*)SetClanTag, &SetClanHook, 7);

	//printf("ct: %X\n", SetClanTag);

	//system("pause");

	//while (true) { 
	//	Sleep(1000);
	//	//origSetClanTag("test", "test");
	//}

	return TRUE;
}



BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, 0);
		//DisableThreadLibraryCalls(hModule);
	}
    return TRUE;
}

