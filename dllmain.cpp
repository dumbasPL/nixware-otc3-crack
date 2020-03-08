// dllmain.cpp : Defines the entry point for the DLL application.
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#include <windows.h>
#include <psapi.h>
#include <winternl.h>


#include "cheat.h"
#include "Classes.h"
#include "XorCompileTime.hpp"
//#include "Hook.h"
#include "detours.h"

#include <wininet.h>
#pragma comment(lib, "wininet.lib")


#define RELOC_FLAG(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_HIGHLOW)


void* CaptureInterface(LPCSTR strModule, LPCSTR strInterface) {
	typedef void* (*CreateInterfaceFn)(const char* szName, int iReturn);
	return ((CreateInterfaceFn)GetProcAddress(GetModuleHandleA(strModule), _("CreateInterface")))(strInterface, 0);
}

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

//void hexDump(void* addr, int len) {
//	int i;
//	unsigned char buff[17];
//	unsigned char* pc = (unsigned char*)addr;
//	if (len == 0) {
//		printf("  ZERO LENGTH\n");
//		return;
//	}
//	if (len < 0) {
//		printf("  NEGATIVE LENGTH: %i\n", len);
//		return;
//	}
//	for (i = 0; i < len; i++) {
//		if ((i % 16) == 0) {
//			if (i != 0) printf("  %s\n", buff);
//			printf("  %04x ", i);
//		}
//		printf(" %02x", pc[i]);
//		if ((pc[i] < 0x20) || (pc[i] > 0x7e)) buff[i % 16] = '.';
//		else buff[i % 16] = pc[i];
//		buff[(i % 16) + 1] = '\0';
//	}
//	while ((i % 16) != 0) {
//		printf("   ");
//		i++;
//	}
//	printf("  %s\n", buff);
//}

/*

loaded
otc loader: 0
connect(45.11.24.138:8505)
recv
  0000  61 64 34 35 35 35 63 38 39 61 61 63 37 66 34 61  ad4555c89aac7f4a
  0010  61 35 32 38 39 30 37 31 33 64 65 39 39 30 65 61  a52890713de990ea
ZwDeviceIoControlFile_hook replaced
send
  0000  63 4d 54 35 2f 66 47 73 49 43 78 67 41 53 53 59  cMT5/fGsICxgASSY
  0010  38 45 45 4f 4b 68 66 5a 4b 76 37 52 37 4b 6c 4c  8EEOKhfZKv7R7KlL
  0020  74 36 36 7a 46 67 4b 4d 57 41 63 77 33 51 38 61  t66zFgKMWAcw3Q8a
  0030  71 66 76 35 71 66 59 64 34 37 57 4a 6e 65 61 50  qfv5qfYd47WJneaP
  0040  31 69 46 58 49 31 46 7a 4c 4a 39 61 56 6a 7a 44  1iFXI1FzLJ9aVjzD
  0050  6c 2f 62 43 69 51 75 6a 39 4c 2b 34 61 74 65 31  l/bCiQuj9L+4ate1
  0060  4e 49 34 50 70 51 56 55 78 62 4d 4c 78 71 44 4d  NI4PpQVUxbMLxqDM
  0070  6f 67 30 58 38 4d 47 2b 70 59 43 76 67 48 72 78  og0X8MG+pYCvgHrx
recv
  0000  72 4e 43 31 34 77 69 7a 51 78 4c 56 39 2b 4b 2f  rNC14wizQxLV9+K/
  0010  38 34 5a 77 39 41 3d 3d                          84Zw9A==

*/

unsigned char hwid[] = {
	0x28, 0x00, 0x00, 0x00, 0x8C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
	0x39, 0x00, 0x00, 0x00, 0x7A, 0x00, 0x00, 0x00, 0x8B, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x41, 0x44, 0x41, 0x54, 0x41, 0x20, 0x53,
	0x58, 0x38, 0x32, 0x30, 0x30, 0x50, 0x4E, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x30, 0x31, 0x31, 0x38, 0x43,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x4A, 0x33, 0x35, 0x32,
	0x30, 0x30, 0x37, 0x31, 0x38, 0x36, 0x34, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5F,
	0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x2E, 0x00,
};

unsigned char server_hello[] = {
	0x61, 0x64, 0x34, 0x35, 0x35, 0x35, 0x63, 0x38, 0x39, 0x61, 0x61, 0x63, 0x37, 0x66, 0x34, 0x61,
	0x61, 0x35, 0x32, 0x38, 0x39, 0x30, 0x37, 0x31, 0x33, 0x64, 0x65, 0x39, 0x39, 0x30, 0x65, 0x61,
};

unsigned char server_response[] = {
	0x72, 0x4e, 0x43, 0x31, 0x34, 0x77, 0x69, 0x7a, 0x51, 0x78, 0x4c, 0x56, 0x39, 0x2b, 0x4b, 0x2f,
	0x38, 0x34, 0x5a, 0x77, 0x39, 0x41, 0x3d, 0x3d,
};

typedef int		(WSAAPI* MySend					)(SOCKET, char*, int, int);
typedef int		(WSAAPI* MyRecv					)(SOCKET, char*, int, int);
typedef int		(WSAAPI* MyConnect				)(SOCKET s, const struct sockaddr * name, int namelen);
typedef NTSTATUS(WINAPI* MyZwDeviceIoControlFile)(HANDLE FileHandle, HANDLE Event, PIO_APC_ROUTINE  ApcRoutine, PVOID ApcContext, 
	PIO_STATUS_BLOCK IoStatusBlock, ULONG IoControlCode, PVOID InputBuffer, ULONG InputBufferLength, PVOID OutputBuffer, ULONG OutputBufferLength);
typedef void	(__fastcall *MySetClanTag		)(const char*, const char*);

MySend					OrigSend					= NULL;
MyRecv					OrigRecv					= NULL;
MyConnect				OrigConnect					= NULL;
MyZwDeviceIoControlFile	OrigZwDeviceIoControlFile	= NULL;
MySetClanTag			OrigSetClanTag				= NULL;

//int WSAAPI SendDetour(SOCKET s, char* buf, int len, int flags) {
//	printf("send\n");
//	hexDump(buf, len);
//	return OrigSend(s, buf, len, flags);
//}
//
//int WINAPI RecvDetour(SOCKET s, char* buf, int len, int flags) {
//	int ret = OrigRecv(s, buf, len, flags);
//	printf("recv\n");
//	hexDump(buf, ret);
//	return ret;
//}

int WSAAPI ConnectDetour(SOCKET s, const struct sockaddr * name, int namelen) {
	struct sockaddr_in* addr_in = (struct sockaddr_in*)name;
	
	addr_in->sin_addr.S_un.S_un_b.s_b1 = 127;
	addr_in->sin_addr.S_un.S_un_b.s_b2 = 0;
	addr_in->sin_addr.S_un.S_un_b.s_b3 = 0;
	addr_in->sin_addr.S_un.S_un_b.s_b4 = 1;

	char* ss = inet_ntoa(addr_in->sin_addr);
	uint16_t port = htons(addr_in->sin_port);
	//printf("connect(%s:%d)\n", ss, port);

	return OrigConnect(s, name, namelen);
}

NTSTATUS WINAPI ZwDeviceIoControlFile_hook(HANDLE FileHandle, HANDLE Event, PIO_APC_ROUTINE  ApcRoutine, PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock, ULONG IoControlCode, PVOID InputBuffer, ULONG InputBufferLength, PVOID OutputBuffer, ULONG OutputBufferLength) {

	NTSTATUS ret = OrigZwDeviceIoControlFile(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock,
		IoControlCode, InputBuffer, InputBufferLength, OutputBuffer, OutputBufferLength);

	if (IoControlCode == IOCTL_STORAGE_QUERY_PROPERTY && OutputBufferLength >= sizeof(hwid)) {
		ZeroMemory(OutputBuffer, OutputBufferLength);
		memcpy(OutputBuffer, hwid, sizeof(hwid));
		//printf("ZwDeviceIoControlFile_hook replaced\n");
		return ret;
	}

	return ret;
}

void CreteEmulatedNixwareServer() {

	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		//printf("WSAStartup failed with error: %d\n", iResult);
		return;
	}

	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo* result = NULL;
	iResult = getaddrinfo(NULL, "8505", &hints, &result);
	if (iResult != 0) {
		//printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return;
	}

	SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		//printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		//printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		//printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		//printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	closesocket(ListenSocket);

	unsigned char rec_buff[512];
	OrigSend(ClientSocket, (char*)server_hello,		sizeof(server_hello),		0);
	OrigRecv(ClientSocket, (char*)rec_buff,			sizeof(rec_buff),			0);
	OrigSend(ClientSocket, (char*)server_response,	sizeof(server_response),	0);
	
	closesocket(ClientSocket);
	WSACleanup();
	return;
}

//const char* getClanTagText(int& timer, int& maxTimer) {
//	std::string baseText = "otc by nezu";
//	maxTimer = (int)baseText.size();
//	if (timer >= maxTimer) timer = 0;
//	std::string s = baseText.substr(timer, maxTimer - timer) + " " + baseText.substr(0, timer);
//	return (s + std::string(14, ' ')).c_str();
//}

//void __fastcall SetClanTagHook(const char* tag, const char* name) {
//	/*static int timer = 0;
//	static int maxTimer = 0;
//	if (timer > maxTimer) timer = 0;
//	const char* clantag = getClanTagText(timer, maxTimer);*/
//	//OrigSetClanTag("otc by nezu", "otc by nezu");
//}

bool is_ru() {
	//using json = nlohmann::json;
	const std::string url = _("https://ipinfo.io/geo");
	auto connect = InternetOpenA(_("GoogleChrome"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (connect) {
		auto address = InternetOpenUrlA(connect, url.c_str(), NULL, 0, INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_KEEP_CONNECTION, 0);
		if (address) {
			char data_received[10000];
			DWORD number_of_bytes_read = 0;
			if (InternetReadFile(address, data_received, sizeof(data_received), &number_of_bytes_read) && number_of_bytes_read) {
				std::string data(data_received);
				std::string cc = data.substr(data.find(_("country")) + strlen(_("country")) + 4, 2);
				return cc.compare(_("RU")) == 0;
			}
		} else {
			InternetCloseHandle(connect);
		}
	}
	return true;
}


BOOL MainThread(HMODULE hModule) {

	/*AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);*/

	//printf(_("loaded\n"));

	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)CreteEmulatedNixwareServer, 0, 0, 0);
	Sleep(200);//just in case

	//system("pause");

	//OrigSend = (MySend)send_hook.HookFunction((void*)&send, &SendDetour, 8);
	//OrigRecv = (MyRecv)recv_hook.HookFunction((void*)&recv, &RecvDetour, 8);
	OrigSend = (MySend)&send;
	OrigRecv = (MyRecv)&recv;
	OrigConnect = (MyConnect)&connect;
	OrigZwDeviceIoControlFile = (MyZwDeviceIoControlFile)GetProcAddress(LoadLibraryA(_("ntdll.dll")), _("ZwDeviceIoControlFile"));
	OrigSetClanTag = (MySetClanTag)FindPattern(_("engine.dll"), _("53 56 57 8B DA 8B F9 FF 15"));
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	//DetourAttach(&(PVOID&)OrigSend, &SendDetour);
	//DetourAttach(&(PVOID&)OrigRecv, &RecvDetour);
	DetourAttach(&(PVOID&)OrigConnect, &ConnectDetour);
	DetourAttach(&(PVOID&)OrigZwDeviceIoControlFile, &ZwDeviceIoControlFile_hook);
	//DetourAttach(&(PVOID&)OrigSetClanTag, &SetClanTagHook);
	
	LONG hook_error = DetourTransactionCommit();
	//printf("otc loader: %d\n", hook_error);

	ICVar * CVar = (ICVar*)CaptureInterface(_("vstdlib.dll"), _("VEngineCvar007"));
	IEngine* Engine = (IEngine*)CaptureInterface(_("engine.dll"), _("VEngineClient014"));
	IGameEventManager* GameEventManager = (IGameEventManager*)CaptureInterface(_("engine.dll"), _("GAMEEVENTSMANAGER002"));

	*(int*)((DWORD)&CVar->FindVar(_("name"))->fnChangeCallback + 0xC) = NULL;
	CVar->FindVar(_("name"))->SetValue(_("cracked by nezu"));

	BYTE* pSrcData = dll_data;

	IMAGE_NT_HEADERS * pOldNtHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(pSrcData + reinterpret_cast<IMAGE_DOS_HEADER*>(pSrcData)->e_lfanew);
	IMAGE_OPTIONAL_HEADER* pOldOptHeader = &pOldNtHeader->OptionalHeader;
	IMAGE_FILE_HEADER* pOldFileHeader = &pOldNtHeader->FileHeader;

	BYTE* pTargetBase = (BYTE*)VirtualAlloc(0, pOldOptHeader->SizeOfImage, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	int error = GetLastError();
	if (!pTargetBase) {
		//printf("Memory allocation failed (ex) 0x%X\n", GetLastError());
		delete[] pSrcData;
		FreeLibraryAndExitThread(hModule, 0);
		return TRUE;
	}

	auto* pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
	for (UINT i = 0; i != pOldFileHeader->NumberOfSections; ++i, ++pSectionHeader) {
		if (pSectionHeader->SizeOfRawData) {
			memcpy(pTargetBase + pSectionHeader->VirtualAddress, pSrcData + pSectionHeader->PointerToRawData, pSectionHeader->SizeOfRawData);
			//printf("Can't map sections: 0x%x\n", GetLastError()); //sucess print
		}
	}
	memcpy(pTargetBase, pSrcData, 0x1000);

	using f_DLL_ENTRY_POINT = BOOL(WINAPI*)(void* hDll, DWORD dwReason, void* pReserved);

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

_("  /$$$$$$                                /$$                       /$$       /$$                \x00"
" /$$__  $$                              | $$                      | $$      | $$                \x00"
"| $$  \\__/  /$$$$$$   /$$$$$$   /$$$$$$$| $$   /$$  /$$$$$$   /$$$$$$$      | $$$$$$$  /$$   /$$\x00"
"| $$       /$$__  $$ |____  $$ /$$_____/| $$  /$$/ /$$__  $$ /$$__  $$      | $$__  $$| $$  | $$\x00"
"| $$      | $$  \\__/  /$$$$$$$| $$      | $$$$$$/ | $$$$$$$$| $$  | $$      | $$  \\ $$| $$  | $$\x00"
"| $$    $$| $$       /$$__  $$| $$      | $$_  $$ | $$_____/| $$  | $$      | $$  | $$| $$  | $$\x00"
"|  $$$$$$/| $$      |  $$$$$$$|  $$$$$$$| $$ \\  $$|  $$$$$$$|  $$$$$$$      | $$$$$$$/|  $$$$$$$\x00"
" \\______/ |__/       \\_______/ \\_______/|__/  \\__/ \\_______/ \\_______/      |_______/  \\____  $$\x00"
"                                                                                       /$$  | $$\x00"
"                                                                                      |  $$$$$$/\x00"
"                                                                                       \\______/ \x00"
"                                                                                                \x00"
"                                                                                                \x00"
"                         /$$$$$$$   /$$$$$$  /$$$$$$$$ /$$   /$$                                \x00"
"                        | $$__  $$ /$$__  $$|____ /$$/| $$  | $$                                \x00"
"                        | $$  \\ $$| $$$$$$$$   /$$$$/ | $$  | $$                                \x00"
"                        | $$  | $$| $$_____/  /$$__/  | $$  | $$                                \x00"
"                        | $$  | $$|  $$$$$$$ /$$$$$$$$|  $$$$$$/                                \x00"
"                        |__/  |__/ \\_______/|________/ \\______/                                 \x00"
"                                                                                                \x00"
" \x00\x00");

	char* art = (char*)asci;

	float h = 0;
	while (*art != 0) {
		h = 0;
		for (size_t i = 0; i < strlen(art); i++) {
			char cha[2] = " ";
			cha[0] = *(art + i);
			CVar->ConsoleColorPrintf(Color::FromHSB(h, 1.f, 1.f), _("%s"), cha);
			h += 0.008f;
			if (h > 1.0) h = 0.f;
		}
		CVar->ConsoleColorPrintf(Color(0, 0, 0), "\n");
		art = art + strlen(art) + 1;
	}


	CVar->ConsoleColorPrintf(Color(0, 255, 0), "\n");

	CVar->ConsoleColorPrintf(Color(0, 255, 0), _("donate(BTC): 37iE16RbjUt7L62LSxoJb2VCmKALK9kjet\n"));
	CVar->ConsoleColorPrintf(Color(0, 255, 0), _("donate(PayPal): https://paypal.me/dumbasPL\n"));
	CVar->ConsoleColorPrintf(Color(0, 255, 0), _("discord(new): https://discord.gg/GWcfEKU\n"));

	//printf("clantag %x\n", (DWORD)OrigSetClanTag);

	/*if(!is_ru()) return true;

	chat_spam::singleton()->initialize(GameEventManager, Engine);*/

	/*while (true) {
		OrigSetClanTag("otc by nezu", "otc by nezu");
		Sleep(100);
	}*/

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

